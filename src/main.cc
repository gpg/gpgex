/* main.cc - DLL entry point
   Copyright (C) 2007 g10 Code GmbH

   This file is part of GpgEX.

   GpgEX is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 
   of the License, or (at your option) any later version.
  
   GpgEX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GpgEX; if not, write to the Free Software Foundation, 
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <windows.h>

#include "registry.h"
#include "gpgex-class.h"
#include "gpgex-factory.h"
#include "main.h"


/* This is the main part of the COM server component.  The component
   is an in-process server DLL.  */

/* The instance of this DLL.  */
HINSTANCE gpgex_server::instance;

/* The number of references to this component.  */
LONG gpgex_server::refcount;


/* Registry key for this software.  */
#define REGKEY "Software\\GNU\\GnuPG"


static char *
get_locale_dir (void)
{
  char *instdir;
  char *p;
  char *dname;

  instdir = read_w32_registry_string ("HKEY_LOCAL_MACHINE", REGKEY,
				      "Install Directory");
  if (!instdir)
    return NULL;
  
  /* Build the key: "<instdir>/share/locale".  */
#define SLDIR "\\share\\locale"
  dname = static_cast<char *> (malloc (strlen (instdir) + strlen (SLDIR) + 1));
  if (!dname)
    {
      free (instdir);
      return NULL;
    }
  p = dname;
  strcpy (p, instdir);
  p += strlen (instdir);
  strcpy (p, SLDIR);
  
  free (instdir);
  
  return dname;
}


static void
drop_locale_dir (char *locale_dir)
{
  free (locale_dir);
}


static void
i18n_init (void)
{
  char *locale_dir;

#ifdef ENABLE_NLS
# ifdef HAVE_LC_MESSAGES
  setlocale (LC_TIME, "");
  setlocale (LC_MESSAGES, "");
# else
  setlocale (LC_ALL, "" );
# endif
#endif

  locale_dir = get_locale_dir ();
  if (locale_dir)
    {
      bindtextdomain (PACKAGE_GT, locale_dir);
      drop_locale_dir (locale_dir);
    }
  textdomain (PACKAGE_GT);
}


/* Entry point called by DLL loader.  */
int WINAPI
DllMain (HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if (reason == DLL_PROCESS_ATTACH)
    {
      gpgex_server::instance = hinst;

      /* FIXME: Initialize subsystems.  */

      /* Early initializations of our subsystems. */
      i18n_init ();
    }
  else if (reason == DLL_PROCESS_DETACH)
    {
    }
  
  return TRUE;
}


/* Check if the server component, the DLL, can be unloaded.  This is
   called by the client of this in-process server (for example through
   CoFreeUnusedLibrary) whenever it is considered to unload this
   server component.  */
STDAPI
DllCanUnloadNow (void)
{
  return (gpgex_server::refcount == 0 ? S_OK : S_FALSE);
}


/* Registry handling.  The DLL is registered with regsvr32.exe.  */

/* Register the DLL.  */
STDAPI
DllRegisterServer (void)
{
  gpgex_class::init ();

  return S_OK;
}


/* Unregister the DLL.  */
STDAPI
DllUnregisterServer (void)
{
  gpgex_class::deinit ();

  return S_OK;
}


/* Acquire a reference for the class factory of object class RCLSID
   with the interface RIID (typically IID_IClassFactory) and return it
   in PPVOUT.  Typically called by CoGetClassObject for in-process
   server components.  */
STDAPI
DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
  if (rclsid == CLSID_gpgex)
    return gpgex_factory.QueryInterface (riid, ppv);

  /* Be nice to broken software.  */
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;

  return 0;
}
