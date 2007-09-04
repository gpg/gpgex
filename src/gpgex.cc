/* gpgex.cc - gpgex implementation
   Copyright (C) 2007 g10 Code GmbH
   
   This file is part of GpgEX.
 
   GpgEX is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   GpgEX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Lesser General Public License for more details.
 
   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <vector>
#include <string>
#include <stdexcept>

using std::vector;
using std::string;

#include <windows.h>

#include "main.h"
#include "client.h"

#include "gpgex.h"


/* Reset the instance between operations.  */
void
gpgex_t::reset (void)
{
  this->filenames.clear ();
  this->all_files_gpg = TRUE;
}


STDMETHODIMP
gpgex_t::QueryInterface (REFIID riid, void **ppv)
{
#define _TRACE_BEG12(a,b,c,d,e,f) TRACE_BEG12(a,b,c,d,e,f)
  _TRACE_BEG12 (DEBUG_INIT, "gpgex_t::QueryInterface", this,
		"riid=" GUID_FMT ", ppv=%p", GUID_ARG (riid), ppv);

  if (ppv == NULL)
    return TRACE_RES (E_INVALIDARG);

  /* Be nice to broken software.  */
  *ppv = NULL;

  /* The static casts ensure that the virtual function table layout of
     the returned object is correct.  We can not cast to IUnknown
     because that base class is ambiguous (because it is not virtual),
     so we pick one of the derived classes instead.  */
  if (riid == IID_IUnknown)
    *ppv = static_cast<IShellExtInit *> (this);
  else if (riid == IID_IShellExtInit)
    *ppv = static_cast<IShellExtInit *> (this);
  else if (riid == IID_IContextMenu)
    *ppv = static_cast<IContextMenu3 *> (this);
#if 0
  /* FIXME: Enable this when the functions are actually
     implemented.  */
  else if (riid == IID_IContextMenu2)
    *ppv = static_cast<IContextMenu3 *> (this);
  else if (riid == IID_IContextMenu3)
    *ppv = static_cast<IContextMenu3 *> (this);
#endif
  else
    return TRACE_RES (E_NOINTERFACE);

  /* We have to acquire a reference to the returned object.  We lost
     the type information, but we know that all object classes inherit
     from IUnknown, which is good enough.  */
  reinterpret_cast<IUnknown *>(*ppv)->AddRef ();

  return TRACE_RES (S_OK);
}


STDMETHODIMP_(ULONG)
gpgex_t::AddRef (void)
{
  (void) TRACE1 (DEBUG_INIT, "gpgex_t::AddRef", this,
		 "new_refcount=%i", this->refcount + 1);

  return InterlockedIncrement (&this->refcount);
}


STDMETHODIMP_(ULONG)
gpgex_t::Release (void)
{
  LONG count;

  (void) TRACE1 (DEBUG_INIT, "gpgex_t::Release", this,
		 "new_refcount=%i", this->refcount - 1);

  count = InterlockedDecrement (&this->refcount);
  if (count == 0)
    delete this;

  return count;
}


/* IShellExtInit methods.  */

STDMETHODIMP
gpgex_t::Initialize (LPCITEMIDLIST pIDFolder, IDataObject *pDataObj, 
		     HKEY hRegKey)
{
  HRESULT err = S_OK;

  TRACE_BEG3 (DEBUG_INIT, "gpgex_t::Initialize", this,
	      "pIDFolder=%p, pDataObj=%p, hRegKey=%p",
	      pIDFolder, pDataObj, hRegKey);

  /* This function is called for the Shortcut (context menu),
     Drag-and-Drop, and Property Sheet extensions.  */

  this->reset ();

  try
    {
      if (pDataObj)
	{ 
	  /* The data object contains a drop item which we extract.  */
	  FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	  STGMEDIUM medium;
	  UINT count;

	  if (SUCCEEDED (pDataObj->GetData (&fe, &medium)))
	    {
	      HDROP drop = (HDROP) GlobalLock (medium.hGlobal);
	      unsigned int i;
	      
	      /* Now that we have the drop item, we can extract the
		 file names.  */
	      count = DragQueryFile (drop, (UINT) -1, NULL, 0);
	      if (count == 0)
		throw std::invalid_argument ("no filenames");

	      try
		{
		  for (i = 0; i < count; i++)
		    {
		      char filename[MAX_PATH];
		      UINT len;
		      len = DragQueryFile (drop, i,
					   filename, sizeof (filename) - 1);
		      if (len == 0)
			throw std::invalid_argument ("zero-length filename");

		      /* Take a look at the ending.  */
		      char *ending = strrchr (filename, '.');
		      if (ending)
			{
			  BOOL gpg = false;

			  ending++;
			  if (! strcasecmp (ending, "gpg")
			      || ! strcasecmp (ending, "pgp")
			      || ! strcasecmp (ending, "asc")
			      || ! strcasecmp (ending, "sig"))
			    gpg = true;
			      
			  if (gpg == false)
			    this->all_files_gpg = FALSE;
			}

		      this->filenames.push_back (filename);
		    }
		}
	      catch (...)
		{
		  GlobalUnlock (medium.hGlobal);
		  ReleaseStgMedium (&medium);
		  throw;
		}

	      GlobalUnlock (medium.hGlobal);
	      ReleaseStgMedium (&medium);
	    }
	}
    }
  catch (std::bad_alloc)
    {
      err = E_OUTOFMEMORY;
    }
  catch (std::invalid_argument &e)
    {
      (VOID) TRACE_LOG1 ("exception: E_INVALIDARG: %s", e.what ());
      err = E_INVALIDARG;
    }
  catch (...)
    {
      err = E_UNEXPECTED;
    }

  if (err != S_OK)
    this->reset ();

  return TRACE_RES (err);
}


/* IContextMenu methods.  */

/* The argument HMENU contains the context menu, and INDEXMENU points
   to the first index where we can add items.  IDCMDFIRST and
   IDCMDLAST is the range of command ID values which we can use.  */
STDMETHODIMP
gpgex_t::QueryContextMenu (HMENU hMenu, UINT indexMenu, UINT idCmdFirst,
			   UINT idCmdLast, UINT uFlags)
{
  BOOL res;

  TRACE_BEG5 (DEBUG_CONTEXT_MENU, "gpgex_t::QueryContextMenu", this,
	      "hMenu=%p, indexMenu=%u, idCmdFirst=%u, idCmdLast=%u, uFlags=%x",
	      hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

  /* FIXME: Do something if idCmdLast - idCmdFirst + 1 is not big
     enough.  */

  /* If the flags include CMF_DEFAULTONLY then nothing should be done.  */
  if (uFlags & CMF_DEFAULTONLY)
    return TRACE_RES (MAKE_HRESULT (SEVERITY_SUCCESS, FACILITY_NULL, 0));

  res = InsertMenu (hMenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
  if (! res)
    return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));
  
  /* First we add the file-specific menus.  */
  if (this->all_files_gpg)
    {
      res = InsertMenu (hMenu, indexMenu++, MF_BYPOSITION | MF_STRING,
			idCmdFirst + ID_CMD_VERIFY_DECRYPT,
			ID_CMD_STR_VERIFY_DECRYPT);
      if (! res)
	return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));
    }
  else
    {
      /* FIXME: Check error.  */
      res = InsertMenu (hMenu, indexMenu++, MF_BYPOSITION | MF_STRING,
			idCmdFirst + ID_CMD_SIGN_ENCRYPT,
			ID_CMD_STR_SIGN_ENCRYPT);
      if (! res)
	return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));
    }

  /* Now generate and add the generic command popup menu.  */
  HMENU popup;
  UINT idx = 0;

  /* FIXME: Check error.  */
  popup = CreatePopupMenu ();
  if (popup == NULL)
    return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));

  res = InsertMenu (hMenu, indexMenu++, MF_BYPOSITION | MF_STRING | MF_POPUP,
		    (UINT) popup, _("More GpgEX options"));
  if (!res)
    {
      DWORD last_error = GetLastError ();
      DestroyMenu (popup);
      return TRACE_RES (HRESULT_FROM_WIN32 (last_error));
    }

  if (this->key_bitmap)
    {
      // indexMenu - 1!!!
      res = SetMenuItemBitmaps (hMenu, indexMenu - 1, MF_BYPOSITION,
				this->key_bitmap, this->key_bitmap);
    }
  if (res)
    res = InsertMenu (hMenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR,
		      0, NULL);
  if (! res)
    return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));

  res = InsertMenu (popup, idx++, MF_BYPOSITION | MF_STRING,
		    idCmdFirst + ID_CMD_VERIFY_DECRYPT,
		    ID_CMD_STR_VERIFY_DECRYPT);

  if (res)
    res = InsertMenu (popup, idx++, MF_BYPOSITION | MF_STRING,
		      idCmdFirst + ID_CMD_SIGN_ENCRYPT,
		      ID_CMD_STR_SIGN_ENCRYPT);
  if (res)
    res = InsertMenu (popup, idx++, MF_BYPOSITION | MF_STRING,
		      idCmdFirst + ID_CMD_IMPORT, ID_CMD_STR_IMPORT);
  if (res)
    res = InsertMenu (popup, idx++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
  if (res)
    res = InsertMenu (popup, idx++, MF_BYPOSITION | MF_STRING,
		      idCmdFirst + ID_CMD_HELP, ID_CMD_STR_HELP);
  if (! res)
    return TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));

  /* We should return a HRESULT that indicates success and the offset
     to the next free command ID after the last one we used, relative
     to idCmdFirst.  In other words: max_used - idCmdFirst + 1.  */
  return TRACE_RES (MAKE_HRESULT (SEVERITY_SUCCESS, FACILITY_NULL,
				  ID_CMD_MAX + 1));
}


/* Get a verb or help text for the command IDCOMMAND (which is the
   offset to IDCMDFIRST of QueryContextMenu, ie zero based).  UFLAGS
   has GCS_HELPTEXT set if the help-text is requested (otherwise a
   verb is requested).  If UFLAGS has the GCS_UNICODE bit set, we need
   to return a wide character string.  */

STDMETHODIMP
gpgex_t::GetCommandString (UINT idCommand, UINT uFlags, LPUINT lpReserved,
			   LPSTR pszName, UINT uMaxNameLen)
{
  TRACE_BEG5 (DEBUG_CONTEXT_MENU, "gpgex_t::GetCommandString", this,
	      "idCommand=%u, uFlags=%x, lpReserved=%lu, pszName=%p, "
	      "uMaxNameLen=%u",
	      idCommand, uFlags, lpReserved, pszName, uMaxNameLen);

  if (idCommand != 0)
    return TRACE_RES (E_INVALIDARG);

  if (! (uFlags & GCS_HELPTEXT))
    return TRACE_RES (E_INVALIDARG);

  if (uFlags & GCS_UNICODE)
    lstrcpynW ((LPWSTR) pszName, L"GpgEX help string (unicode)", uMaxNameLen);
  else
    lstrcpynA (pszName, "GpgEX help string (ASCII)", uMaxNameLen);

  return TRACE_RES (S_OK);
}


STDMETHODIMP
gpgex_t::InvokeCommand (LPCMINVOKECOMMANDINFO lpcmi)
{
  TRACE_BEG1 (DEBUG_CONTEXT_MENU, "gpgex_t::InvokeCommand", this,
	      "lpcmi=%p", lpcmi);

  /* If lpVerb really points to a string, ignore this function call
     and bail out.  */
  if (HIWORD (lpcmi->lpVerb) != 0)
    return TRACE_RES (E_INVALIDARG);
 
  client_t client (lpcmi->hwnd);

  /* Get the command index, which is the offset to IDCMDFIRST of
     QueryContextMenu, ie zero based).  */
  switch (LOWORD (lpcmi->lpVerb))
    {
    case ID_CMD_HELP:
      {
	string msg;
	unsigned int i;
	
	msg = "Invoked Help on files:\n\n";
	for (i = 0; i < this->filenames.size (); i++)
	  msg = msg + this->filenames[i] + '\n';
	
	MessageBox (lpcmi->hwnd, msg.c_str (), "GpgEX", MB_ICONINFORMATION);
      }
      break;
    case ID_CMD_VERIFY_DECRYPT:
      client.decrypt_verify (this->filenames);
      break;
    case ID_CMD_SIGN_ENCRYPT:
      client.encrypt_sign (this->filenames);
      break;
    case ID_CMD_IMPORT:
      client.import (this->filenames);
      break;
    default:
      return TRACE_RES (E_INVALIDARG);
      break;
    }

  return TRACE_RES (S_OK);
}


/* IContextMenu2 methods.  */

STDMETHODIMP
gpgex_t::HandleMenuMsg (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  /* FIXME */
  return S_OK;
}


/* IContextMenu3 methods.  */

STDMETHODIMP
gpgex_t::HandleMenuMsg2 (UINT uMsg, WPARAM wParam, LPARAM lParam,
			     LRESULT *plResult)
{
  /* FIXME */
  return S_OK;
}
