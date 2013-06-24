/* main.h - main prototypes
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

#ifndef MAIN_H
#define MAIN_H	1

#include <windows.h>
#include "debug.h"


#include <gpg-error.h>  /* For gettext */

#define _(a) gettext (a)
#define N_(a) gettext_noop (a)

/* A pseudo function call that serves as a marker for the automated
   extraction of messages, but does not call gettext().  The run-time
   translation is done at a different place in the code.
   The argument, String, should be a literal string.  Concatenated strings
   and other string expressions won't work.
   The macro's expansion is not parenthesized, so that it is suitable as
   initializer for static 'char[]' or 'const char[]' variables.  */
#define gettext_noop(String) String



/* We use a class just for namespace cleanliness.  */
class gpgex_server
{
 public:
  /* The instance of this DLL.  */
  static HINSTANCE instance;

  /* Global reference counting for the server component.  This is
     increased by acquiring references to any COM objects as well as
     when locking the server component, and needed to implement
     DllCanUnloadNow.  */

  /* The number of references to this component.  */
  static LONG refcount;

  /* Acquire a reference to the server component.  */
  static inline ULONG
    add_ref (void)
  {
    return InterlockedIncrement (&refcount);
  }


/* Release a reference to the server component.  */
  static inline ULONG
    release (void)
  {
    return InterlockedDecrement (&refcount);
  }
};


#define GUID_FMT "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}"
#define GUID_ARG(x) (x).Data1, (x).Data2, (x).Data3, (x).Data4[0], \
    (x).Data4[1], (x).Data4[2], (x).Data4[3], (x).Data4[4],	   \
    (x).Data4[5], (x).Data4[6], (x).Data4[7]

#endif
