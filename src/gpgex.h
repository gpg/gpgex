/* gpgex.h - gpgex prototypes
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

#ifndef GPGEX_H
#define GPGEX_H

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <windows.h>
#include <shlobj.h>

#include "bitmaps.h"

/* For context menus.  */
#define ID_CMD_HELP		0
#define ID_CMD_VERIFY_DECRYPT	1
#define ID_CMD_SIGN_ENCRYPT	2
#define ID_CMD_IMPORT		3
#define ID_CMD_MAX		3

#define ID_CMD_STR_HELP			_("Help on GpgEX")
#define ID_CMD_STR_VERIFY_DECRYPT	_("Decrypt and verify")
#define ID_CMD_STR_SIGN_ENCRYPT		_("Sign and encrypt")
#define ID_CMD_STR_IMPORT		_("Import keys")


/* Our shell extension interface.  We use multiple inheritance to
   achieve polymorphy.  

   NOTE 1: By this we save some effort, but we can only provide one
   implementation for each virtual function signature.  The overlap in
   the IUnknown interface does not matter, in fact it is a plus that
   we only have to implement it once.  For other functions, it might
   be more of a problem.  If this needs to be avoided, one can derive
   intermediate classes which inherit only one of the overlapping
   classes and contain a implementations for the overlapping method
   that call a purely virtual function of different, unambiguous
   names.  For example, if there is bar::foo and baz::foo, classes
   mybar : public bar and mybaz : public baz can be defined with
   mybar::foo calling mybar::bar_foo and mybaz::foo calling
   mybaz::baz_foo.  Then the final class can inherit mybar and mybaz
   and implement the virtual functions bar_foo and baz_foo, leading to
   the desired result.

   NOTE 2: It is not obvious why this approach works at all!
   Ignorance is bliss, I guess, because the multiple-inheritance
   approach is documented in many places, but rarely it is explained
   why it works.  The naive explanation is that there is a virtual
   function table for each base class, and we can just use the address
   of the pointer to that table as our COM object pointer.  However,
   what is missing from this description is that now the THIS pointer
   is incorrect, and needs to be adjusted by subtracting the offset of
   the base class inside the object when a function implementation is
   invoked (which exists in the derived class and overrides the
   abstract base class).  Recent compilers seem to implement this by
   replacing the function pointer in the VTBL with an "adjustor thunk"
   which subtracts this offset and jumps to the actual function
   implementation (see the C++ ABI for GCC
   http://www.codesourcery.com/cxx-abi/abi.html and
   http://msdn.microsoft.com/archive/default.asp?url=/archive/en-us/dnarvc/html/jangrayhood.asp
   for MSVC++).  But this is not the only possible implementation:
   Other compilers use a displacement field in each VTBL entry.  The
   displacement field is often zero, and in this case an adjustor
   thunk is a good optimization.  However, if a displacement field is
   used by the compiler, it changes the VTBL layout in memory and
   makes it non-compliant with COM!  So let's all be happy that modern
   compilers agree on using adjustor thunks and cross our fingers, as
   there is no possible way we can even sanely check if the compiler
   complies.  And you thought C++ was easy?  */

class gpgex_t : public IShellExtInit, public IContextMenu3
{
 private:
  /* Per-object reference count.  */
  LONG refcount;

  /* Support for IShellExtInit.  */
  vector<string> filenames;
  
  /* TRUE if all files in filenames are directly related to GPG.  */
  BOOL all_files_gpg;

  /* Support for the context menu.  */
  HBITMAP key_bitmap;

 public:
  /* Constructors and destructors.  For these, we update the global
     component reference counter.  */
  gpgex_t (void)
    : refcount (0)
    {
      TRACE_BEG (DEBUG_INIT, "gpgex_t::gpgex_t", this);

      gpgex_server::add_ref ();

      this->key_bitmap = gpgex_bitmaps.load_bitmap ("Key");

      (void) TRACE_SUC ();
    }

  /* The "virtual" fixes a compile time warning.  */
  virtual ~gpgex_t (void)
    {
      TRACE_BEG (DEBUG_INIT, "gpgex_t::~gpgex_t", this);

      if (this->key_bitmap != NULL)
	DeleteObject (this->key_bitmap);

      gpgex_server::release ();

      (void) TRACE_SUC ();
    }

  /* Reset the instance between operations.  */
  void reset (void);

 public:
  /* IUnknown methods.  */
  STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
  STDMETHODIMP_(ULONG) AddRef (void);
  STDMETHODIMP_(ULONG) Release (void);

  /* IShellExtInit methods.  */
  STDMETHODIMP Initialize (LPCITEMIDLIST pIDFolder, IDataObject *pDataObj, 
			   HKEY hRegKey);

  /* IContextMenu methods.  */
  STDMETHODIMP QueryContextMenu (HMENU hMenu, UINT indexMenu, UINT idCmdFirst,
				 UINT idCmdLast, UINT uFlags);
  STDMETHODIMP GetCommandString (UINT idCommand, UINT uFlags, LPUINT lpReserved,
				 LPSTR pszName, UINT uMaxNameLen);
  STDMETHODIMP InvokeCommand (LPCMINVOKECOMMANDINFO lpcmi);

  /* IContextMenu2 methods.  */
  STDMETHODIMP HandleMenuMsg (UINT uMsg, WPARAM wParam, LPARAM lParam);

  /* IContextMenu3 methods.  */
  STDMETHODIMP HandleMenuMsg2 (UINT uMsg, WPARAM wParam, LPARAM lParam,
			       LRESULT *plResult);
};

#endif	/* ! GPGEX_H */
