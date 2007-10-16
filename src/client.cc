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

#include "main.h"
#include "registry.h"
#include "exechelp.h"

#include "client.h"


static const char *
default_uiserver_name (void)
{
  static string name;

  if (name.size () == 0)
    {
      char *dir = NULL;

      dir = read_w32_registry_string ("HKEY_LOCAL_MACHINE", REGKEY,
				      "Install Directory");
      if (dir)
	{
	  char *uiserver = NULL;
	  int uiserver_malloced = 1;
	  
	  uiserver = read_w32_registry_string (NULL, REGKEY, "UI Server");
	  if (!uiserver)
	    {
	      uiserver = "bin\\kleopatra.exe";
	      uiserver_malloced = 0;
	    }

	  /* FIXME: Very dirty work-around to make kleopatra find its
	     DLLs.  */
	  if (strcmp (uiserver, "bin\\kleopatra.exe"))
	    chdir (dir);

	  try { name = ((string) dir) + "\\" + uiserver; } catch (...) {}

	  if (uiserver_malloced)
	    free (uiserver);
	  free ((void *) dir);
	}
    }

  return name.c_str ();
}

static const char *
default_socket_name (void)
{
  static string name;

  if (name.size () == 0)
    {
      char *dir = NULL;
      
      dir = default_homedir ();
      if (dir)
	{
	  try { name = ((string) dir) + "\\S.uiserver"; } catch (...) {}
	  free ((void *) dir);
	}
    }

  return name.c_str ();
}


#define tohex_lower(n) ((n) < 10 ? ((n) + '0') : (((n) - 10) + 'a'))

/* Percent-escape the string STR by replacing colons with '%3a'.  If
   EXTRA is not NULL all characters in it are also escaped. */
static char *
percent_escape (const char *str, const char *extra)
{
  int i, j;
  char *ptr;

  if (!str)
    return NULL;

  for (i=j=0; str[i]; i++)
    if (str[i] == ':' || str[i] == '%' || (extra && strchr (extra, str[i])))
      j++;
  ptr = (char *) malloc (i + 2 * j + 1);
  i = 0;
  while (*str)
    {
      /* FIXME: Work around a bug in Kleo.  */
      if (*str == ':')
	{
	  ptr[i++] = '%';
	  ptr[i++] = '3';
	  ptr[i++] = 'a';
	}
      else
    if (*str == '%')
	{
	  ptr[i++] = '%';
	  ptr[i++] = '2';
	  ptr[i++] = '5';
	}
      else if (extra && strchr (extra, *str))
        {
	  ptr[i++] = '%';
          ptr[i++] = tohex_lower ((*str >> 4) & 15);
          ptr[i++] = tohex_lower (*str & 15);
        }
      else
	ptr[i++] = *str;
      str++;
    }
  ptr[i] = '\0';

  return ptr;
}


static string
escape (string str)
{
  char *arg_esc = percent_escape (str.c_str (), "+= ");

  if (arg_esc == NULL)
    throw std::bad_alloc ();

  string res = arg_esc;
  free (arg_esc);

  return res;
}


static int
uiserver_connect (assuan_context_t *ctx)
{
  int rc;
  const char *socket_name = NULL;

  TRACE_BEG (DEBUG_ASSUAN, "client_t::uiserver_connect", ctx);

  socket_name = default_socket_name ();
  if (! socket_name || ! *socket_name)
    {
      (void) TRACE_LOG ("invalid socket name");
      return TRACE_GPGERR (gpg_error (GPG_ERR_INV_ARG));
    }

  (void) TRACE_LOG1 ("socket name: %s", socket_name);
  rc = assuan_socket_connect (ctx, socket_name, -1);
  if (rc)
    {
      const char *argv[3];
      int count;

      (void) TRACE_LOG ("UI server not running, starting it");

      argv[0] = "--uiserver-socket";
      argv[1] = socket_name;
      argv[2] = NULL;

      rc = gpgex_spawn_detached (default_uiserver_name (), argv);
      if (rc)
	return TRACE_GPGERR (rc);

      /* Give it a bit of time to start up and try a couple of
	 times.  */
      for (count = 0; count < 10; count++)
	{
	  Sleep (1000);
	  rc = assuan_socket_connect (ctx, socket_name, -1);
	  if (!rc)
	    break;
	}
    }
  return TRACE_GPGERR (rc);
}


bool
client_t::call_assuan (const char *cmd, vector<string> &filenames)
{
  int rc = 0;
  assuan_context_t ctx = NULL;
  string msg;
  
  TRACE_BEG2 (DEBUG_ASSUAN, "client_t::call_assuan", this,
	      "%s on %u files", cmd, filenames.size ());

  rc = uiserver_connect (&ctx);
  if (rc)
    goto leave;

  try
    {
      /* Set the input files.  We don't specify the output files.  */
      for (unsigned int i = 0; i < filenames.size (); i++)
	{
	  msg = "INPUT FILE=" + escape (filenames[i]);
	  
	  (void) TRACE_LOG1 ("sending cmd: %s", msg.c_str ());
	  
	  rc = assuan_transact (ctx, msg.c_str (),
				NULL, NULL, NULL, NULL, NULL, NULL);
	  if (rc)
	    goto leave;
	}
      
      /* Set the --nohup option, so that the operation continues and
	 completes in the background.  */
      msg = ((string) cmd) + " --nohup";
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
  this->call_assuan ("ENCRYPT_SIGN", filenames);
}


void
client_t::encrypt (vector<string> &filenames)
{
  this->call_assuan ("ENCRYPT", filenames);
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


void
client_t::create_checksums (vector<string> &filenames)
{
  this->call_assuan ("CREATE_CHECKSUMS", filenames);
}


void
client_t::verify_checksums (vector<string> &filenames)
{
  this->call_assuan ("VERIFY_CHECKSUMS", filenames);
}
