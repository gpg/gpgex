/* main.cc - DLL entry point
   Copyright (C) 2007, 2010, 2013 g10 Code GmbH

   This file is part of GpgEX.

   GpgEX is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   GpgEX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>

#include <gpg-error.h>
#include <assuan.h>

#include "gpgex-class.h"
#include "gpgex-factory.h"
#include "main.h"


/* This is the main part of the COM server component.  The component
   is an in-process server DLL.  */

/* The instance of this DLL.  */
HINSTANCE gpgex_server::instance;

/* The number of references to this component.  */
LONG gpgex_server::refcount;

/* The name of the UI-server or NULL if not known.  */
const char *gpgex_server::ui_server;



/** Get the Gpg4win Install directory.
 *
 * This function returns the root of the install directory of Gpg4win
 * or GnuPG-[VS-]Desktop.  Noet that the fucntion returns only
 * standard slashes.
 *
 * @returns NULL if no dir could be found - this indicates a bad
 * installation.  Otherwise a malloced string with the utf-8 name.
 */
const char *
get_gpg4win_dir (void)
{
  static int initialized;
  static char *mydir;

  if (!initialized)
    {
      wchar_t *wmodulename;
      char *modulename = NULL;
      char *p;

      wmodulename = (wchar_t*)calloc (MAX_PATH+5, sizeof *wmodulename);
      if (!GetModuleFileNameW (gpgex_server::instance, wmodulename, MAX_PATH))
        _gpgex_debug (DEBUG_INIT, "GetModuleFileName failed");
      else
        {
          modulename = gpgrt_wchar_to_utf8 (wmodulename);
          _gpgex_debug (DEBUG_INIT, "GetModuleFileName: '%s'\n", modulename);
          p = strrchr (modulename, '\\');
          if (p)
            {
              *p = 0;
              /* MODULENAME is now the actual directory from where we
               * were executed.  In the most cases this is either the
               * bin or the bin_64 sub directory.  */
              p = strrchr (modulename, '\\');
              if (p)
                {
                  *p = 0;
                  for (p=modulename; *p; p++)
                    if (*p == '\\')
                      *p = '/';
                  mydir = modulename;  /* Not really threadsafe, but okay.  */
                  modulename = NULL;
                  initialized = 1;
                  _gpgex_debug (DEBUG_INIT, "Using install dir: '%s'\n", mydir);
                }
            }
        }
      free (modulename);
      free (wmodulename);
    }

  return mydir;
}


static char *
get_locale_dir (void)
{
  return gpgrt_fconcat (0, get_gpg4win_dir (), "share/locale", NULL);
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
unsigned int debug_flags = 0;

/* Debug log file.  */
FILE *debug_file;


/* Get the filename of the debug file, if any.  */
static char *
get_debug_file (void)
{
  return gpgrt_w32_reg_get_string ("\\Software\\Gpg4win:GpgEX Debug File");
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






#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* Log the formatted string FORMAT at debug level LEVEL or higher.  */
extern
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


#ifdef __cplusplus
#if 0
{
#endif
}
#endif


/* Entry point called by DLL loader.  */
STDAPI
DllMain (HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if (reason == DLL_PROCESS_ATTACH)
    {
      gpgex_server::instance = hinst;

      /* Early initializations of our subsystems. */
      gpg_err_init ();

      debug_init ();

      i18n_init ();

      if (debug_flags & DEBUG_ASSUAN)
	{
	  assuan_set_assuan_log_stream (debug_file);
	  assuan_set_assuan_log_prefix ("gpgex:assuan");
	}
      assuan_set_gpg_err_source (GPG_ERR_SOURCE_DEFAULT);

      (void) TRACE0 (DEBUG_INIT, "DllMain", hinst,
		     "reason=DLL_PROCESS_ATTACH");

      {
	WSADATA wsadat;

	WSAStartup (0x202, &wsadat);
      }
    }
  else if (reason == DLL_PROCESS_DETACH)
    {
      WSACleanup ();

      (void) TRACE0 (DEBUG_INIT, "DllMain", hinst,
		     "reason=DLL_PROCESS_DETACH");

      debug_deinit ();
      /* We are linking statically to libgpg-error which means there
         is no DllMain in libgpg-error.  Thus we call the deinit
         function to cleanly deinitialize libgpg-error.  */
      gpg_err_deinit (0);
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
