/* gpgex-class.cc - gpgex component class implementation
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

#include "gpgex-class.h"


/* The class ID in a form that can be used by certain interfaces.  */
CLSID CLSID_gpgex = CLSID_GPGEX;


/* Because we do not use a type library (.tlb) resource file, we have
   to do all of the work manually.  However, that's not a big issue,
   because we only register a new component class, and no custom
   interfaces.  So, for example, we do not need to register proxy/stub
   DLLs.  */

/* Register the GpgEX component.  */
void
gpgex_class::init (void)
{
  DWORD result;
  char key[MAX_PATH];
  char value[MAX_PATH];
  HKEY key_handle = 0;

  /* FIXME: Error handling?  */

  strcpy (key, "CLSID\\{" CLSID_GPGEX_STR "}");
  RegCreateKey (HKEY_CLASSES_ROOT, key, &key_handle);

  /* The default value is a human readable name for the class.  */
  strcpy (value, "GpgEX");
  RegSetValueEx (key_handle, 0, 0, REG_SZ, (BYTE *) value, strlen (value) + 1);
  RegCloseKey (key_handle);

  /* The InprocServer32 key holds the path to the server component.  */
  strcpy (key, "CLSID\\{" CLSID_GPGEX_STR "}\\InprocServer32");
  RegCreateKey (HKEY_CLASSES_ROOT, key, &key_handle);

  result = GetModuleFileName (gpgex_server::instance, value, MAX_PATH);
  RegSetValueEx (key_handle, 0, 0, REG_SZ, (BYTE *) value, strlen (value) + 1);
  RegCloseKey (key_handle);

  /* FIXME: Now add the required registry keys for the context menu
     triggers.  */
}


/* Unregister the GpgEX component.  */
void
gpgex_class::deinit (void)
{
  /* FIXME: Error handling?  */

  /* FIXME: Unregister the required registry keys for the context menu
     triggers.  */

  /* Delete registry keys in reverse order.  */
  RegDeleteKey (HKEY_CLASSES_ROOT,
		"CLSID\\{" CLSID_GPGEX_STR "}\\InprocServer32");
  RegDeleteKey (HKEY_CLASSES_ROOT, "CLSID\\{" CLSID_GPGEX_STR "}");
}
