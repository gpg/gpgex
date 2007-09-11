/* client.h - gpgex assuan client interface
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

#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <windows.h>

class client_t
{
 private:
  HWND window;

  bool call_assuan (const char *cmd, vector<string> &filenames);

 public:
  client_t (HWND window_handle)
    : window (window_handle)
  {
  }
  
  void decrypt_verify (vector<string> &filenames);
  void decrypt (vector<string> &filenames);
  void verify (vector<string> &filenames);
  void encrypt_sign (vector<string> &filenames);
  void encrypt (vector<string> &filenames);
  void sign (vector<string> &filenames);
  void import (vector<string> &filenames);
};

#endif	/* ! CLIENT_H */
