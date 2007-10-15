/* registry.h - registry prototypes
   Copyright (C) 2006, 2007 g10 Code GmbH

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

#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* This is a helper function to load a Windows function from either of
   one DLLs. */
HRESULT w32_shgetfolderpath (HWND a, int b, HANDLE c, DWORD d, LPSTR e);

/* Return a string from the Win32 Registry or NULL in case of error.
   Caller must release the return value.  A NULL for root is an alias
   for HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE in turn.  */
char *read_w32_registry_string (const char *root, const char *dir,
				const char *name);

/* Retrieve the default home directory.  */
char *default_homedir (void);

/* Registry key for this software.  */
#define REGKEY "Software\\GNU\\GnuPG"

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif	/* ! REGISTRY_H */
