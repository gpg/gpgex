/* bitmaps.cc - gpgex bitmap implementation
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

#include <string>
#include <sstream>

using std::string;

#include <windows.h>

#include "main.h"

#include "bitmaps.h"


/* The size of the icons.  */
int gpgex_bitmaps_t::size;

/* The available bitmap sizes in ascending order.  */
int gpgex_bitmaps_t::available_sizes[] = { 12, 16 };


/* The global singleton object.  */
class gpgex_bitmaps_t gpgex_bitmaps;


gpgex_bitmaps_t::gpgex_bitmaps_t (void)
{
  /* Note about bitmaps: The required size is given by
     GetSystemMetrics and can vary depending on the display size.  A
     typical value is 12x12.  The color depth should be 8 bits.  The
     upper left corner pixel color is replaced by transparent
     automatically.  */
  int width = GetSystemMetrics (SM_CXMENUCHECK);
  int height = GetSystemMetrics (SM_CYMENUCHECK);
  
  /* All our images are square, so take the minimum and look for the
     biggest available size that fits in there.  */
  int max_size = (width < height) ? width : height;

  for (unsigned int i = 0; i < (sizeof (this->available_sizes)
				/ sizeof (this->available_sizes[0])); i++)
    if (max_size >= this->available_sizes[i])
      this->size = this->available_sizes[i];
    else
      break;
  
  (void) TRACE3 (DEBUG_INIT, "gpgex_bitmaps_t::gpgex_bitmaps_t", this,
		 "GetSystemMetrics: %ix%i (using %i)", width, height,
		 this->size);
}


/* Load the bitmap with name NAME.  */
HBITMAP gpgex_bitmaps_t::load_bitmap (string name)
{
  HBITMAP bmap;
  std::ostringstream out;

  out << name << "-" << this->size;
  bmap = LoadBitmap (gpgex_server::instance, out.str().c_str());
  if (bmap == NULL)
    (void) TRACE2 (DEBUG_INIT, "gpgex_bitmaps_t::load_bitmap", this,
		   "LoadImage %s failed: ec=%x",
		   out.str().c_str(), GetLastError ());
  else
    (void) TRACE1 (DEBUG_INIT, "gpgex_bitmaps_t::load_bitmap", this,
		   "loaded image %s", out.str().c_str());
    
  /* FIXME: Create cache of images.  */
  return bmap;
}

