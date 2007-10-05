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

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>

#include <gpg-error.h>
#include <assuan.h>

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


static CRITICAL_SECTION debug_lock;

/* No flags on means no debugging.  */
static unsigned int debug_flags = 0;

static FILE *debug_file;


/* Get the filename of the debug file, if any.  */
static char *
get_debug_file (void)
{
  return read_w32_registry_string ("HKEY_LOCAL_MACHINE", REGKEY,
				   "GpgEX Debug File");
}


static void
debug_init (void)
{
  char *filename;

  /* Sanity check.  */
  if (debug_file)
    return;

  InitializeCriticalSection (&debug_lock);  

  filename = get_debug_file ();
  if (!filename)
    return;

  debug_file = fopen (filename, "a");
  free (filename);
  if (!debug_file)
    return;

  /* FIXME: Make this configurable eventually.  */
  debug_flags = DEBUG_INIT | DEBUG_CONTEXT_MENU | DEBUG_ASSUAN;
}


static void
debug_deinit (void)
{
  if (debug_file)
    {
      fclose (debug_file);
      debug_file = NULL;
    }
}


/* Log the formatted string FORMAT at debug level LEVEL or higher.  */
void
_gpgex_debug (unsigned int flags, const char *format, ...)
{
  va_list arg_ptr;
  int saved_errno;

  saved_errno = errno;

  if (! (debug_flags & flags))
    return;
      
  va_start (arg_ptr, format);
  EnterCriticalSection (&debug_lock);
  vfprintf (debug_file, format, arg_ptr);
  va_end (arg_ptr);
  if (format && *format && format[strlen (format) - 1] != '\n')
    putc ('\n', debug_file);
  LeaveCriticalSection (&debug_lock);
  fflush (debug_file);

  errno = saved_errno;
}


/* Entry point called by DLL loader.  */
STDAPI
DllMain (HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if (reason == DLL_PROCESS_ATTACH)
    {
      gpgex_server::instance = hinst;

      /* Early initializations of our subsystems. */
      i18n_init ();

      debug_init ();

      if (debug_flags & DEBUG_ASSUAN)
	{
	  assuan_set_assuan_log_stream (debug_file);
	  assuan_set_assuan_log_prefix ("gpgex:assuan");
	}
      assuan_set_assuan_err_source (GPG_ERR_SOURCE_DEFAULT);

      (void) TRACE0 (DEBUG_INIT, "DllMain", hinst,
		     "reason=DLL_PROCESS_ATTACH");

      {
	WSADATA wsadat;
	
	WSAStartup (0x202, &wsadat);
      }
    }
  else if (reason == DLL_PROCESS_DETACH)
    {
      (void) TRACE0 (DEBUG_INIT, "DllMain", hinst,
		     "reason=DLL_PROCESS_DETACH");

      debug_deinit ();
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
  (void) TRACE (DEBUG_INIT, "DllCanUnloadNow", gpgex_server::refcount);
  return (gpgex_server::refcount == 0 ? S_OK : S_FALSE);
}


/* Registry handling.  The DLL is registered with regsvr32.exe.  */

/* Register the DLL.  */
STDAPI
DllRegisterServer (void)
{
  gpgex_class::init ();

  /* Notify the shell about the change.  */
  SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

  return S_OK;
}


/* Unregister the DLL.  */
STDAPI
DllUnregisterServer (void)
{
  gpgex_class::deinit ();

  /* Notify the shell about the change.  */
  SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

  return S_OK;
}


/* Acquire a reference for the class factory of object class RCLSID
   with the interface RIID (typically IID_IClassFactory) and return it
   in PPVOUT.  Typically called by CoGetClassObject for in-process
   server components.  */
STDAPI
DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
  /* We have to evaluate the arguments first.  */
#define _TRACE_BEG22(a,b,c,d,e,f) TRACE_BEG22(a,b,c,d,e,f)
  _TRACE_BEG22 (DEBUG_INIT, "DllGetClassObject", ppv,
		"rclsid=" GUID_FMT ", riid=" GUID_FMT,
		GUID_ARG (rclsid), GUID_ARG (riid));

  if (rclsid == CLSID_gpgex)
    {
      HRESULT err = gpgex_factory.QueryInterface (riid, ppv);
      return TRACE_RES (err);
    }

  /* Be nice to broken software.  */
  *ppv = NULL;

  return TRACE_RES (CLASS_E_CLASSNOTAVAILABLE);
}
