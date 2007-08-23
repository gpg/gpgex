/* gpgex-class.h - gpgex prototypes
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

#ifndef GPGEX_CLASS_H
#define GPGEX_CLASS_H

#include <windows.h>


/* We need our own COM class, because the COM class identifier (an
   UUID) is what is used to register the server component with the
   Windows Explorer Shell.  The shell will then reference the
   extension by this CLSID.  */
#define CLSID_GPGEX_STR "CCD955E4-5C16-4A33-AFDA-A8947A94946B"
#define CLSID_GPGEX { 0xccd955e4, 0x5c16, 0x4a33,		\
      { 0xaf, 0xda, 0xa8, 0x94, 0x7a, 0x94, 0x94, 0x6b } };

/* The class ID in a form that can be used by certain interfaces.  */
extern CLSID CLSID_gpgex;

/* We do not use custom interfaces.  This also spares us from
   implementing and registering a proxy/stub DLL.  */


class gpgex_class
{
 public:
  /* Unregister the GpgEX component.  */
  static void init (void);

  /* Unregister the GpgEX component.  */
  static void deinit (void);
};

#endif	/* ! GPGEX_CLASS_H */
