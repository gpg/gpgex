/* exechelp.h - fork and exec helpers
 * Copyright (C) 2004, 2007 g10 Code GmbH
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

#ifndef GPGEX_EXECHELP_H
#define GPGEX_EXECHELP_H

#include <gpg-error.h>

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* Fork and exec the PGMNAME with /dev/null as stdin, stdout and
   stderr.  The arguments for the process are expected in the NULL
   terminated array ARGV.  The program name itself should not be
   included there.  Returns 0 on success or an error code.  */
gpg_error_t gpgex_spawn_detached (const char *pgmname,
				  const char *const argv[]);
#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif /* GPGEX_EXECHELP_H */
