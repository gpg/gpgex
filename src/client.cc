/* client.cc - gpgex assuan client implementation
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

#include <vector>
#include <string>
#include <stdexcept>

using std::vector;
using std::string;

#include <windows.h>

#include <assuan.h>

#include "registry.h"
#include "main.h"

#include "client.h"


static const char *
default_socket_name (void)
{
  static string name;

  if (name.size () == 0)
    {
      char *dir = NULL;
      
      /* FIXME: Wrong directory.  */
      dir = default_homedir ();
      if (dir)
	{
	  try { name = ((string) dir) + "\\S.uiserver"; } catch (...) {}
	  free ((void *) dir);
	}
    }

  return name.c_str ();
}


bool
client_t::call_assuan (const char *cmd, vector<string> &filenames)
{
  int rc = 0;
  assuan_context_t ctx = NULL;
  const char *socket_name;
  string msg;
  
  TRACE_BEG2 (DEBUG_ASSUAN, "client_t::call_assuan", this,
	      "%s on %u files", cmd, filenames.size ());

  socket_name = default_socket_name ();
  if (! socket_name || ! *socket_name)
    {
      (void) TRACE_LOG ("invalid socket name");
      rc = gpg_error (GPG_ERR_INV_ARG);
      goto leave;
    }

  (void) TRACE_LOG1 ("socket name: %s", socket_name);
  rc = assuan_socket_connect (&ctx, socket_name, -1);
  if (rc)
    goto leave;

  try
    {
      /* Set the input files.  FIXME: Might need to set the output files
	 as well.  */
      for (unsigned int i = 0; i < filenames.size (); i++)
	{
	  msg = "INPUT FILE=\"" + filenames[i] + "\" --continued";
	  
	  (void) TRACE_LOG1 ("sending cmd: %s", msg.c_str ());
	  
	  rc = assuan_transact (ctx, msg.c_str (),
				NULL, NULL, NULL, NULL, NULL, NULL);
	  if (rc)
	    goto leave;
	}
      
      /* Set the --nohup option, so that the operation continues and
	 completes in the background.  */
      msg = "OPTION --nohup";
      (void) TRACE_LOG1 ("sending cmd: %s", msg.c_str ());
      rc = assuan_transact (ctx, msg.c_str (),
			    NULL, NULL, NULL, NULL, NULL, NULL);
      if (rc)
	goto leave;
      
      msg = cmd;
      (void) TRACE_LOG1 ("sending cmd: %s", msg.c_str ());
      rc = assuan_transact (ctx, msg.c_str (),
			    NULL, NULL, NULL, NULL, NULL, NULL);
    }
  catch (std::bad_alloc)
    {
      rc = gpg_error (GPG_ERR_ENOMEM);
    }
  catch (...)
    {
      rc = gpg_error (GPG_ERR_GENERAL);
    }
  
  /* Fall-through.  */
 leave:
  TRACE_GPGERR (rc);
  if (ctx)
    assuan_disconnect (ctx);
  if (rc)
    {
      MessageBox (this->window,
		  _("Can not access Kleopatra, see log file for details"),
		  "GpgEX", MB_ICONINFORMATION);
    }

  return rc ? false : true;
}


void
client_t::decrypt_verify (vector<string> &filenames)
{
  this->call_assuan ("DECRYPT_VERIFY", filenames);
}


void
client_t::verify (vector<string> &filenames)
{
  this->call_assuan ("VERIFY", filenames);
}


void
client_t::decrypt (vector<string> &filenames)
{
  this->call_assuan ("DECRYPT", filenames);
}


void
client_t::encrypt_sign (vector<string> &filenames)
{
  this->call_assuan ("ENCRPYT_SIGN", filenames);
}


void
client_t::encrypt (vector<string> &filenames)
{
  this->call_assuan ("ENCRPYT", filenames);
}


void
client_t::sign (vector<string> &filenames)
{
  this->call_assuan ("SIGN", filenames);
}


void
client_t::import (vector<string> &filenames)
{
  this->call_assuan ("IMPORT", filenames);
}
