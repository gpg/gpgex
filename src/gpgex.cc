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

#include <windows.h>

#include "main.h"

#include "gpgex.h"


STDMETHODIMP
gpgex_t::QueryInterface (REFIID riid, void **ppv)
{
  if (ppv == NULL)
    return E_INVALIDARG;

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
    return E_NOINTERFACE;  

  /* We have to acquire a reference to the returned object.  We lost
     the type information, but we know that all object classes inherit
     from IUnknown, which is good enough.  */
  reinterpret_cast<IUnknown *>(*ppv)->AddRef ();

  return S_OK;
}


STDMETHODIMP_(ULONG)
gpgex_t::AddRef (void)
{
  return InterlockedIncrement (&this->refcount);
}


STDMETHODIMP_(ULONG)
gpgex_t::Release (void)
{
  LONG count;

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
  /* FIXME */
  return S_OK;
}


/* IContextMenu methods.  */

STDMETHODIMP
gpgex_t::QueryContextMenu (HMENU hMenu, UINT indexMenu, UINT idCmdFirst,
			   UINT idCmdLast, UINT uFlags)
{
  /* FIXME */
  return S_OK;
}


STDMETHODIMP
gpgex_t::GetCommandString (UINT idCommand, UINT uFlags, LPUINT lpReserved,
			   LPSTR pszName, UINT uMaxNameLen)
{
  /* FIXME */
  return S_OK;
}


STDMETHODIMP
gpgex_t::InvokeCommand (LPCMINVOKECOMMANDINFO lpcmi)
{
  /* FIXME */
  return S_OK;
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
