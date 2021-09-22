/* exechelp.c - fork and exec helpers
 * Copyright (C) 2004, 2007, 2014 g10 Code GmbH
 *
 * This file is part of GpgEX.
 *
 * GpgEX is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GpgEX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <windows.h>

#include <gpg-error.h>

#include "debug.h"
#include "exechelp.h"

/* Define to 1 do enable debugging.  */
#define DEBUG_W32_SPAWN 0


struct private_membuf_s
{
  size_t len;
  size_t size;
  char *buf;
  int out_of_core;
};

typedef struct private_membuf_s membuf_t;





/* A simple implementation of a dynamic buffer.  Use init_membuf() to
   create a buffer, put_membuf to append bytes and get_membuf to
   release and return the buffer.  Allocation errors are detected but
   only returned at the final get_membuf(), this helps not to clutter
   the code with out of core checks.  */
void
init_membuf (membuf_t *mb, int initiallen)
{
  mb->len = 0;
  mb->size = initiallen;
  mb->out_of_core = 0;
  mb->buf = malloc (initiallen);
  if (!mb->buf)
    mb->out_of_core = errno;
}


/* Shift the content of the membuf MB by AMOUNT bytes.  The next
   operation will then behave as if AMOUNT bytes had not been put into
   the buffer.  If AMOUNT is greater than the actual accumulated
   bytes, the membuf is basically reset to its initial state.  */
void
clear_membuf (membuf_t *mb, size_t amount)
{
  /* No need to clear if we are already out of core.  */
  if (mb->out_of_core)
    return;
  if (amount >= mb->len)
    mb->len = 0;
  else
    {
      mb->len -= amount;
      memmove (mb->buf, mb->buf+amount, mb->len);
    }
}


void
put_membuf (membuf_t *mb, const void *buf, size_t len)
{
  if (mb->out_of_core || !len)
    return;

  if (mb->len + len >= mb->size)
    {
      char *p;

      mb->size += len + 1024;
      p = realloc (mb->buf, mb->size);
      if (!p)
        {
          mb->out_of_core = errno ? errno : ENOMEM;
          /* /\* Wipe out what we already accumulated.  This is required */
          /*    in case we are storing sensitive data here.  The membuf */
          /*    API does not provide another way to cleanup after an */
          /*    error. *\/ */
          /* wipememory (mb->buf, mb->len); */
          return;
        }
      mb->buf = p;
    }
  if (buf)
    memcpy (mb->buf + mb->len, buf, len);
  else
    memset (mb->buf + mb->len, 0, len);
  mb->len += len;
}


void *
get_membuf (membuf_t *mb, size_t *len)
{
  char *p;

  if (mb->out_of_core)
    {
      if (mb->buf)
        {
          /* wipememory (mb->buf, mb->len); */
          free (mb->buf);
          mb->buf = NULL;
        }
      gpg_err_set_errno (mb->out_of_core);
      return NULL;
    }

  p = mb->buf;
  if (len)
    *len = mb->len;
  mb->buf = NULL;
  mb->out_of_core = ENOMEM; /* hack to make sure it won't get reused. */
  return p;
}



/* Lock a spawning process.  The caller needs to provide the address
   of a variable to store the lock information and the name or the
   process.  */
gpg_error_t
gpgex_lock_spawning (lock_spawn_t *lock)
{
  int waitrc;
  int timeout = 5;

  _TRACE (DEBUG_ASSUAN, "gpgex_lock_spawning", lock);

  *lock = CreateMutexW (NULL, FALSE, L"spawn_gnupg_uiserver_sentinel");
  if (!*lock)
    {
      TRACE_LOG1 ("failed to create the spawn mutex: rc=%d", GetLastError ());
      return gpg_error (GPG_ERR_GENERAL);
    }

 retry:
  waitrc = WaitForSingleObject (*lock, 1000);
  if (waitrc == WAIT_OBJECT_0)
    return 0;

  if (waitrc == WAIT_TIMEOUT && timeout)
    {
      timeout--;
      goto retry;
    }
  if (waitrc == WAIT_TIMEOUT)
    TRACE_LOG ("error waiting for the spawn mutex: timeout");
  else
    TRACE_LOG2 ("error waiting for the spawn mutex: (code=%d) rc=%d",
                waitrc, GetLastError ());
  return gpg_error (GPG_ERR_GENERAL);
}


/* Unlock the spawning process.  */
void
gpgex_unlock_spawning (lock_spawn_t *lock)
{
  if (*lock)
    {
      _TRACE (DEBUG_ASSUAN, "gpgex_unlock_spawning", lock);

      if (!ReleaseMutex (*lock))
        TRACE_LOG1 ("failed to release the spawn mutex: rc=%d", GetLastError());
      CloseHandle (*lock);
      *lock = NULL;
    }
}


/* Fork and exec the program with /dev/null as stdin, stdout and
   stderr.  Returns 0 on success or an error code.  */
gpg_error_t
gpgex_spawn_detached (const char *pgmname, const char *cmdline)
{
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi =
    {
      NULL,      /* Returns process handle.  */
      0,         /* Returns primary thread handle.  */
      0,         /* Returns pid.  */
      0          /* Returns tid.  */
    };
  STARTUPINFO si;
  int cr_flags;

  TRACE_BEG1 (DEBUG_ASSUAN, "gpgex_spawn_detached", cmdline,
	      "cmdline=%s", cmdline);

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr);
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  /* Start the process.  Note that we can't run the PREEXEC function
     because this would change our own environment. */
  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = DEBUG_W32_SPAWN ? SW_SHOW : SW_MINIMIZE;

  cr_flags = (CREATE_DEFAULT_ERROR_MODE
              | GetPriorityClass (GetCurrentProcess ())
	      | CREATE_NEW_PROCESS_GROUP
              | DETACHED_PROCESS);

  if (!CreateProcess (pgmname,         /* pgmname; Program to start.  */
                      (char *) cmdline, /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      cr_flags,      /* Creation flags.  */
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                      ))
    {
      (void) TRACE_LOG1 ("CreateProcess failed: %i\n", GetLastError ());
      return gpg_error (GPG_ERR_GENERAL);
    }

  /* Process has been created suspended; resume it now. */
  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);

  return 0;
}


/* Fork and exec PGMNAME with args in CMDLINE and /dev/null connected
 * to stdin and stderr.  Read from stdout and return the result as a
 * malloced string at R_STRING.  Returns 0 on success or an error code.  */
gpg_error_t
gpgex_spawn_get_string (const char *pgmname, const char *cmdline,
                        char **r_string)
{
  SECURITY_ATTRIBUTES sec_attr;
  HANDLE rh, wh;
  PROCESS_INFORMATION pi =
    {
      NULL,      /* Returns process handle.  */
      0,         /* Returns primary thread handle.  */
      0,         /* Returns pid.  */
      0          /* Returns tid.  */
    };
  STARTUPINFO si;
  membuf_t mb;

  *r_string = NULL;

  TRACE_BEG1 (DEBUG_ASSUAN, "gpgex_spawn_get_string", cmdline,
	      "cmdline=%s", cmdline);

  /* Set the inherit flag for the pipe into the securit attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr);
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = TRUE;

  /* Create a pipe to read form stdout.  */
  if (!CreatePipe (&rh, &wh, &sec_attr, 0))
    {
      TRACE_LOG1 ("CreatePipe failed: ec=%d", (int) GetLastError ());
      return gpg_error (GPG_ERR_GENERAL);
    }

  /* Set the read end to non-inheritable.  */
  if (!SetHandleInformation (rh, HANDLE_FLAG_INHERIT, 0))
    {
      TRACE_LOG1 ("SHI failed: ec=%d", (int) GetLastError ());
      CloseHandle (rh);
      CloseHandle (wh);
      return gpg_error (GPG_ERR_GENERAL);
    }

  /* Start the process.  Note that we can't run the PREEXEC function
     because this would change our own environment. */
  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;
  si.hStdInput  = INVALID_HANDLE_VALUE;
  si.hStdOutput = wh;
  si.hStdError  = INVALID_HANDLE_VALUE;

  if (!CreateProcess (pgmname,       /* pgmname; Program to start.  */
                      (char *)cmdline, /* Command line arguments.  */
                      NULL,          /* Process security attributes.  */
                      NULL,          /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      0,             /* Creation flags.  */
                      NULL,          /* Use current environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                      ))
    {
      (void) TRACE_LOG1 ("CreateProcess failed: %i\n", GetLastError ());
      CloseHandle (rh);
      CloseHandle (wh);
      return gpg_error (GPG_ERR_GENERAL);
    }

  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);

  CloseHandle (wh);  /* This end is used by the child.  */

  init_membuf (&mb, 1024);
  for (;;)
    {
      char readbuf[1024];
      DWORD nread;

      if (!ReadFile (rh, readbuf, sizeof readbuf, &nread, NULL) || !nread)
        break;
      put_membuf (&mb, readbuf, nread);
    }
  CloseHandle (rh);  /* Ready with reading.  */

  put_membuf (&mb, "", 1);  /* Terminate string.  */
  *r_string = get_membuf (&mb, NULL);
  if (!*r_string)
    return gpg_error (GPG_ERR_EIO);

  return 0;
}
