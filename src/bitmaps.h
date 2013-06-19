/* bitmaps.h - gpgex bitmap prototypes
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

#ifndef GPGEX_BITMAPS_H
#define GPGEX_BITMAPS_H

#include <string>

using std::string;

#include <windows.h>


/* The class used to load bitmap resources.  */
class gpgex_bitmaps_t
{
  /* The icon size used.  */
  static int size;

  /* The available sizes.  */
  static int available_sizes[];

 public:
  /* Constructor.  */
  gpgex_bitmaps_t (void);

  /* Load the bitmap with name NAME.  */
  HBITMAP load_bitmap (string name);
};


/* The global singleton object.  */
extern gpgex_bitmaps_t gpgex_bitmaps;

#endif	/* ! GPGEX_BITMAPS_H */
