/* registry.c - Registry routines
   Copyright (C) 2005, 2007 g10 Code GmbH

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
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <windows.h>

#include <shlobj.h>
#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 0x001a
#endif
#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA 0x001c
#endif
#ifndef CSIDL_FLAG_CREATE
#define CSIDL_FLAG_CREATE 0x8000
#endif

#include "registry.h"


/* This is a helper function to load a Windows function from either of
   one DLLs. */
HRESULT
w32_shgetfolderpath (HWND a, int b, HANDLE c, DWORD d, LPSTR e)
{
  static int initialized;
  static HRESULT (WINAPI * func)(HWND,int,HANDLE,DWORD,LPSTR);

  if (!initialized)
    {
      static char *dllnames[] = { "shell32.dll", "shfolder.dll", NULL };
      void *handle;
      int i;

      initialized = 1;

      for (i=0, handle = NULL; !handle && dllnames[i]; i++)
        {
          handle = LoadLibrary (dllnames[i]);
          if (handle)
            {
              func = (HRESULT (WINAPI *)(HWND,int,HANDLE,DWORD,LPSTR))
                     GetProcAddress (handle, "SHGetFolderPathA");
              if (!func)
                {
                  FreeLibrary (handle);
                  handle = NULL;
                }
            }
        }
    }

  if (func)
    return func (a,b,c,d,e);
  else
    return -1;
}


/* Helper for read_w32_registry_string(). */
static HKEY
get_root_key(const char *root)
{
  HKEY root_key;

  if( !root )
    root_key = HKEY_CURRENT_USER;
  else if( !strcmp( root, "HKEY_CLASSES_ROOT" ) )
    root_key = HKEY_CLASSES_ROOT;
  else if( !strcmp( root, "HKEY_CURRENT_USER" ) )
    root_key = HKEY_CURRENT_USER;
  else if( !strcmp( root, "HKEY_LOCAL_MACHINE" ) )
    root_key = HKEY_LOCAL_MACHINE;
  else if( !strcmp( root, "HKEY_USERS" ) )
    root_key = HKEY_USERS;
  else if( !strcmp( root, "HKEY_PERFORMANCE_DATA" ) )
    root_key = HKEY_PERFORMANCE_DATA;
  else if( !strcmp( root, "HKEY_CURRENT_CONFIG" ) )
    root_key = HKEY_CURRENT_CONFIG;
  else
    return NULL;
  return root_key;
}


/* Return a string from the Win32 Registry or NULL in case of error.
   Caller must release the return value.  A NULL for root is an alias
   for HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE in turn.  */
char *
read_w32_registry_string (const char *root, const char *dir, const char *name)
{
  HKEY root_key, key_handle;
  DWORD n1, nbytes, type;
  char *result = NULL;

  if ( !(root_key = get_root_key(root) ) )
    return NULL;

  if( RegOpenKeyEx( root_key, dir, 0, KEY_READ, &key_handle ) )
    {
      if (root)
	return NULL; /* no need for a RegClose, so return direct */
      /* It seems to be common practise to fall back to HKLM. */
      if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, &key_handle) )
	return NULL; /* still no need for a RegClose, so return direct */
    }

  nbytes = 1;
  if( RegQueryValueEx( key_handle, name, 0, NULL, NULL, &nbytes ) ) {
    if (root)
      goto leave;
    /* Try to fallback to HKLM also vor a missing value.  */
    RegCloseKey (key_handle);
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, &key_handle) )
      return NULL; /* Nope.  */
    if (RegQueryValueEx( key_handle, name, 0, NULL, NULL, &nbytes))
      goto leave;
  }
  result = malloc( (n1=nbytes+1) );
  if( !result )
    goto leave;
  if( RegQueryValueEx( key_handle, name, 0, &type, result, &n1 ) ) {
    free(result); result = NULL;
    goto leave;
  }
  result[nbytes] = 0; /* make sure it is really a string  */
  if (type == REG_EXPAND_SZ && strchr (result, '%')) {
    char *tmp;

    n1 += 1000;
    tmp = malloc (n1+1);
    if (!tmp)
      goto leave;
    nbytes = ExpandEnvironmentStrings (result, tmp, n1);
    if (nbytes && nbytes > n1) {
      free (tmp);
      n1 = nbytes;
      tmp = malloc (n1 + 1);
      if (!tmp)
	goto leave;
      nbytes = ExpandEnvironmentStrings (result, tmp, n1);
      if (nbytes && nbytes > n1) {
	free (tmp); /* oops - truncated, better don't expand at all */
	goto leave;
      }
      tmp[nbytes] = 0;
      free (result);
      result = tmp;
    }
    else if (nbytes) { /* okay, reduce the length */
      tmp[nbytes] = 0;
      free (result);
      result = malloc (strlen (tmp)+1);
      if (!result)
	result = tmp;
      else {
	strcpy (result, tmp);
	free (tmp);
      }
    }
    else {  /* error - don't expand */
      free (tmp);
    }
  }

 leave:
  RegCloseKey( key_handle );
  return result;
}


/* Get the standard home directory.  In general this function should
   not be used as it does not consider a registry value (under W32) or
   the GNUPGHOME encironment variable.  It is better to use
   default_homedir(). */
static char *
standard_homedir (void)
{
  static char *dir;

  if (!dir)
    {
      char path[MAX_PATH];
      
      /* It might be better to use LOCAL_APPDATA because this is
         defined as "non roaming" and thus more likely to be kept
         locally.  For private keys this is desired.  However, given
         that many users copy private keys anyway forth and back,
         using a system roaming services might be better than to let
         them do it manually.  A security conscious user will anyway
         use the registry entry to have better control.  */
      if (w32_shgetfolderpath (NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
                               NULL, 0, path) >= 0) 
        {
          char *tmp = malloc (strlen (path) + 6 + 1);

	  strcpy (tmp, path);
	  strcat (tmp, "\\gnupg");

          dir = tmp;
          
          /* Try to create the directory if it does not yet exists.  */
          if (access (dir, F_OK))
            CreateDirectory (dir, NULL);
        }
      else
        dir = "C:\\gnupg";
    }
  return dir;
}


/* Retrieve the default home directory.  */
char *
default_homedir (void)
{
  char *dir;

  dir = getenv ("GNUPGHOME");
  if (!dir || !*dir)
    {
      static char *saved_dir;
      
      if (!saved_dir)
        {
          if (!dir || !*dir)
            {
              char *tmp;

              tmp = read_w32_registry_string (NULL, "Software\\GNU\\GnuPG",
                                              "HomeDir");
              if (tmp && !*tmp)
                {
                  free (tmp);
                  tmp = NULL;
                }
              if (tmp)
                saved_dir = tmp;
            }
          
          if (!saved_dir)
            saved_dir = standard_homedir ();
        }
      dir = saved_dir;
    }
  if (!dir || !*dir)
    dir = "C:\\gnupg";

  return dir;
}
