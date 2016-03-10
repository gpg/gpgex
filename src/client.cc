/* client.cc - gpgex assuan client implementation
   Copyright (C) 2007, 2008, 2013, 2014 g10 Code GmbH

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

#include <winsock2.h>
#include <windows.h>

#include <assuan.h>

#include "main.h"
#include "registry.h"
#include "exechelp.h"

#include "client.h"

static inline char *
_gpgex_stpcpy (char *a, const char *b)
{
  while (*b)
    *a++ = *b++;
  *a = 0;
  return a;
}
#define stpcpy(a,b) _gpgex_stpcpy ((a), (b))



static const char *
default_socket_name (void)
{
  static char *name;

  if (!name)
    {
      const char *dir;
      const char sockname[] = "\\S.uiserver";

      dir = default_homedir ();
      if (dir)
	{
          name = (char *)malloc (strlen (dir) + strlen (sockname) + 1);
          if (name)
            {
              strcpy (name, dir);
              strcat (name, sockname);
            }
	}
    }

  return name;
}


/* Return the name of the default UI server.  This name is used to
   auto start an UI server if an initial connect failed.  */
static const char *
default_uiserver_cmdline (void)
{
  static char *name;

  if (!name)
#if ENABLE_GPA_ONLY
    {
      const char gpaserver[] = "bin\\launch-gpa.exe";
      const char *dir;
      char *p;

      dir = gpgex_server::root_dir;
      if (!dir)
        return NULL;

      name = (char*)malloc (strlen (dir) + strlen (gpaserver) + 9 + 2);
      if (!name)
        return NULL;
      strcpy (stpcpy (stpcpy (name, dir), "\\"), gpaserver);
      for (p = name; *p; p++)
        if (*p == '/')
          *p = '\\';
      strcat (name, " --daemon");
      gpgex_server::ui_server = "GPA";
    }
#else /*!ENABLE_GPA_ONLY*/
    {
      const char *dir, *tmp;
      char *uiserver, *p;
      int extra_arglen = 9;
      const char * server_names[] = {"bin\\kleopatra.exe",
                                     "kleopatra.exe",
                                     "bin\\launch-gpa.exe",
                                     "launch-gpa.exe",
                                     "bin\\gpa.exe",
                                     "gpa.exe",
                                     NULL};

      dir = gpgex_server::root_dir;
      if (!dir)
        return NULL;

      uiserver = read_w32_registry_string (NULL, GPG4WIN_REGKEY_2,
                                           "UI Server");
      if (!uiserver)
        {
          uiserver = read_w32_registry_string (NULL, GPG4WIN_REGKEY_3,
                                               "UI Server");
        }
      if (!uiserver)
        {
          uiserver = strdup ("kleopatra.exe");
          if (!uiserver)
            return NULL;
        }
      if (uiserver)
        {
          name = (char*) malloc (strlen (dir) + strlen (uiserver) +
                                 extra_arglen + 2);
          if (!name)
            return NULL;
          strcpy (stpcpy (stpcpy (name, dir), "\\"), uiserver);
          for (p = name; *p; p++)
            if (*p == '/')
              *p = '\\';
          free (uiserver);
        }
      if (name && !access (name, F_OK))
        {
          /* Set through registry or default kleo */
          if (strstr (name, "kleopatra.exe"))
            {
              gpgex_server::ui_server = "Kleopatra";
              strcat (name, " --daemon");
            }
          else
            {
              gpgex_server::ui_server = "GPA";
            }
          return name;
        }
      /* Fallbacks */
      for (tmp = *server_names; *tmp; tmp++)
        {
          if (name)
            {
              free (name);
            }
          name = (char*) malloc (strlen (dir) + strlen (tmp) + extra_arglen + 2);
          if (!name)
            return NULL;
          strcpy (stpcpy (stpcpy (name, dir), "\\"), tmp);
          for (p = name; *p; p++)
            if (*p == '/')
              *p = '\\';
          if (!access (name, F_OK))
            {
              /* Found a viable candidate */
              /* Set through registry and is accessible */
              if (strstr (name, "kleopatra.exe"))
                {
                  gpgex_server::ui_server = "Kleopatra";
                  strcat (name, " --daemon");
                }
              else
                {
                  gpgex_server::ui_server = "GPA";
                }
              return name;
            }
        }
      gpgex_server::ui_server = NULL;
    }
#endif /*!ENABLE_GPA_ONLY*/

  return name;
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


/* Send options to the UI server and return the server's PID.  */
static gpg_error_t
send_one_option (assuan_context_t ctx, const char *name, const char *value)
{
  gpg_error_t err;
  char buffer[1024];

  if (! value || ! *value)
    err = 0;  /* Avoid sending empty strings.  */
  else
    {
      snprintf (buffer, sizeof (buffer), "OPTION %s=%s", name, value);
      err = assuan_transact (ctx, buffer, NULL, NULL, NULL, NULL, NULL, NULL);
    }

  return err;
}


static gpg_error_t
getinfo_pid_cb (void *opaque, const void *buffer, size_t length)
{
  pid_t *pid = (pid_t *) opaque;

  *pid = (pid_t) strtoul ((char *) buffer, NULL, 10);

  return 0;
}


static gpg_error_t
send_options (assuan_context_t ctx, HWND hwnd, pid_t *r_pid)
{
  gpg_error_t rc = 0;
  char numbuf[50];

  TRACE_BEG (DEBUG_ASSUAN, "client_t::send_options", ctx);

  *r_pid = (pid_t) (-1);
  rc = assuan_transact (ctx, "GETINFO pid", getinfo_pid_cb, r_pid,
			NULL, NULL, NULL, NULL);
  if (! rc && *r_pid == (pid_t) (-1))
    {
      (void) TRACE_LOG ("server did not return a PID");
      rc = gpg_error (GPG_ERR_ASSUAN_SERVER_FAULT);
    }

  if (! rc && *r_pid != (pid_t) (-1)
      && ! AllowSetForegroundWindow (*r_pid))
    {
      (void) TRACE_LOG ("AllowSetForegroundWindow (%u) failed");
      TRACE_RES (HRESULT_FROM_WIN32 (GetLastError ()));

      /* Ignore the error, though.  */
    }

  if (! rc && hwnd)
    {
      /* We hope that HWND is limited to 32 bit.  If not a 32 bit
         UI-server would not be able to do anything with this
         window-id.  */
      uintptr_t tmp = (uintptr_t)hwnd;

      if (!(tmp & ~0xffffffff))
        {
          /* HWND fits into 32 bit - send it. */
          snprintf (numbuf, sizeof (numbuf), "%lx", (unsigned long)tmp);
          rc = send_one_option (ctx, "window-id", numbuf);
        }
    }

  return TRACE_GPGERR (rc);
}


static gpg_error_t
uiserver_connect (assuan_context_t *ctx, HWND hwnd)
{
  gpg_error_t rc;
  const char *socket_name = NULL;
  pid_t pid;
  lock_spawn_t lock;

  TRACE_BEG (DEBUG_ASSUAN, "client_t::uiserver_connect", ctx);

  socket_name = default_socket_name ();
  if (! socket_name || ! *socket_name)
    {
      (void) TRACE_LOG ("invalid socket name");
      return TRACE_GPGERR (gpg_error (GPG_ERR_INV_ARG));
    }

  (void) TRACE_LOG1 ("socket name: %s", socket_name);
  rc = assuan_new (ctx);
  if (rc)
    {
      (void) TRACE_LOG ("could not allocate context");
      return TRACE_GPGERR (rc);
    }

  rc = assuan_socket_connect (*ctx, socket_name, -1, 0);
  if (rc)
    {
      int count;

      (void) TRACE_LOG ("UI server not running, starting it");

      /* Now try to connect again with the spawn lock taken.  */
      if (!(rc = gpgex_lock_spawning (&lock))
          && assuan_socket_connect (*ctx, socket_name, -1, 0))
        {
          rc = gpgex_spawn_detached (default_uiserver_cmdline ());
          if (!rc)
            {
              /* Give it a bit of time to start up and try a couple of
                 times.  */
              for (count = 0; count < 10; count++)
                {
                  Sleep (1000);
                  rc = assuan_socket_connect (*ctx, socket_name, -1, 0);
                  if (!rc)
                    break;
                }
            }

        }
      gpgex_unlock_spawning (&lock);
    }

  if (! rc)
    {
      if (debug_flags & DEBUG_ASSUAN)
	assuan_set_log_stream (*ctx, debug_file);

      rc = send_options (*ctx, hwnd, &pid);
      if (rc)
	{
	  assuan_release (*ctx);
	  *ctx = NULL;
	}
    }

  return TRACE_GPGERR (rc);
}

typedef struct async_arg
{
  const char *cmd;
  vector<string> filenames;
  HWND wid;
} async_arg_t;

static DWORD WINAPI
call_assuan_async (LPVOID arg)
{
  async_arg_t *async_args = (async_arg_t *)arg;
  int rc = 0;
  int connect_failed = 0;
  const char *cmd = async_args->cmd;
  const vector<string> filenames = async_args->filenames;

  assuan_context_t ctx = NULL;
  string msg;

  TRACE_BEG2 (DEBUG_ASSUAN, "client_t::call_assuan_async", 0,
              "%s on %u files", cmd, filenames.size ());

  rc = uiserver_connect (&ctx, async_args->wid);
  if (rc)
    {
      connect_failed = 1;
      goto leave;
    }

  try
    {
      /* Set the input files.  We don't specify the output files.  */
      for (unsigned int i = 0; i < filenames.size (); i++)
        {
          msg = "FILE " + escape (filenames[i]);

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
    assuan_release (ctx);
  if (rc)
    {
      char buf[256];

      if (connect_failed)
        snprintf (buf, sizeof (buf),
                  _("Can not connect to the GnuPG user interface%s%s%s:\r\n%s"),
                  gpgex_server::ui_server? " (":"",
                  gpgex_server::ui_server? gpgex_server::ui_server:"",
                  gpgex_server::ui_server? ")":"",
                  gpg_strerror (rc));
      else
        snprintf (buf, sizeof (buf),
                  _("Error returned by the GnuPG user interface%s%s%s:\r\n%s"),
                  gpgex_server::ui_server? " (":"",
                  gpgex_server::ui_server? gpgex_server::ui_server:"",
                  gpgex_server::ui_server? ")":"",
                  gpg_strerror (rc));
      MessageBox (async_args->wid, buf, "GpgEX", MB_ICONINFORMATION);
    }
  delete async_args;
  return 0;
}

void
client_t::call_assuan (const char *cmd, vector<string> &filenames)
{
  TRACE_BEG (DEBUG_ASSUAN, "client_t::call_assuan", cmd);
  async_arg_t * args = new async_arg_t;
  args->cmd = cmd;
  args->filenames = filenames;
  args->wid = this->window;

  /* We move the call in a different thread as the Windows explorer
     is blocked until our call finishes. We don't want that.
     Additionally Kleopatra / Qt5 SendsMessages to the parent
     window provided in wid. Qt does this with blocking calls
     so Kleopatra blocks until the explorer processes more
     Window Messages and we block the explorer. This is
     a deadlock. */
  CreateThread (NULL, 0, call_assuan_async, (LPVOID) args, 0,
                NULL);
  return;
}


void
client_t::decrypt_verify (vector<string> &filenames)
{
  this->call_assuan ("DECRYPT_VERIFY_FILES", filenames);
}


void
client_t::verify (vector<string> &filenames)
{
  this->call_assuan ("VERIFY_FILES", filenames);
}


void
client_t::decrypt (vector<string> &filenames)
{
  this->call_assuan ("DECRYPT_FILES", filenames);
}


void
client_t::sign_encrypt (vector<string> &filenames)
{
  this->call_assuan ("ENCRYPT_SIGN_FILES", filenames);
}


void
client_t::encrypt (vector<string> &filenames)
{
  this->call_assuan ("ENCRYPT_FILES", filenames);
}


void
client_t::sign (vector<string> &filenames)
{
  this->call_assuan ("SIGN_FILES", filenames);
}


void
client_t::import (vector<string> &filenames)
{
  this->call_assuan ("IMPORT_FILES", filenames);
}


void
client_t::create_checksums (vector<string> &filenames)
{
  this->call_assuan ("CHECKSUM_CREATE_FILES", filenames);
}


void
client_t::verify_checksums (vector<string> &filenames)
{
  this->call_assuan ("CHECKSUM_VERIFY_FILES", filenames);
}
