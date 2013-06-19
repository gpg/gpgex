/* debug.h - trace prototypes
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

#ifndef DEBUG_H
#define DEBUG_H	1

#include <gpg-error.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif


#define DEBUG_INIT		1
#define DEBUG_CONTEXT_MENU	2
#define DEBUG_ASSUAN		4

/* No flags on means no debugging.  */
extern unsigned int debug_flags;

/* Debug log stream.  */
extern FILE *debug_file;


#define STRINGIFY(v) #v

/* Log the formatted string FORMAT in categories FLAGS.  */
void _gpgex_debug (unsigned int flags, const char *format, ...);

#define _TRACE(lvl, name, tag)					\
  int _gpgex_trace_level = lvl;					\
  const char *const _gpgex_trace_func = name;			\
  const char *const _gpgex_trace_tagname = STRINGIFY (tag);	\
  void *_gpgex_trace_tag = (void *) tag

#define TRACE_BEG(lvl, name, tag)			     \
  _TRACE (lvl, name, tag);				     \
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter\n", \
		_gpgex_trace_func, _gpgex_trace_tagname,     \
		_gpgex_trace_tag)
#define TRACE_BEG0(lvl, name, tag, fmt)					\
  _TRACE (lvl, name, tag);						\
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n",	\
		_gpgex_trace_func, _gpgex_trace_tagname,		\
		_gpgex_trace_tag)
#define TRACE_BEG1(lvl, name, tag, fmt, arg1)				\
  _TRACE (lvl, name, tag);						\
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n",	\
		_gpgex_trace_func, _gpgex_trace_tagname,		\
		_gpgex_trace_tag, arg1)
#define TRACE_BEG2(lvl, name, tag, fmt, arg1, arg2)		      \
  _TRACE (lvl, name, tag);					      \
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n", \
		_gpgex_trace_func, _gpgex_trace_tagname,	      \
		_gpgex_trace_tag, arg1, arg2)
#define TRACE_BEG3(lvl, name, tag, fmt, arg1, arg2, arg3)	      \
  _TRACE (lvl, name, tag);					      \
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n", \
		_gpgex_trace_func, _gpgex_trace_tagname,	      \
		_gpgex_trace_tag, arg1, arg2, arg3)
#define TRACE_BEG4(lvl, name, tag, fmt, arg1, arg2, arg3, arg4)	      \
  _TRACE (lvl, name, tag);					      \
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n", \
		_gpgex_trace_func, _gpgex_trace_tagname,	      \
		_gpgex_trace_tag, arg1, arg2, arg3, arg4)
#define TRACE_BEG5(lvl, name, tag, fmt, arg1, arg2, arg3, arg4, arg5) \
  _TRACE (lvl, name, tag);					      \
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n", \
		_gpgex_trace_func, _gpgex_trace_tagname,	      \
		_gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5)
#define TRACE_BEG12(lvl, name, tag, fmt, arg1, arg2, arg3, arg4,	\
		    arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
  _TRACE (lvl, name, tag);						\
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n",	\
		_gpgex_trace_func, _gpgex_trace_tagname,		\
		_gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5, arg6,	\
		arg7, arg8, arg9, arg10, arg11, arg12)
#define TRACE_BEG13(lvl, name, tag, fmt, arg1, arg2, arg3, arg4,	\
		    arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12,	\
		    arg13)						\
  _TRACE (lvl, name, tag);						\
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n",	\
		_gpgex_trace_func, _gpgex_trace_tagname,		\
		_gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5, arg6,	\
		arg7, arg8, arg9, arg10, arg11, arg12, arg13)
#define TRACE_BEG22(lvl, name, tag, fmt, arg1, arg2, arg3, arg4,	\
		   arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12,	\
		   arg13, arg14, arg15, arg16, arg17, arg18, arg19,	\
		   arg20, arg21, arg22)					\
  _TRACE (lvl, name, tag);						\
  _gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): enter: " fmt "\n",	\
		_gpgex_trace_func, _gpgex_trace_tagname,		\
		_gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5, arg6,	\
		arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14,	\
		arg15, arg16, arg17, arg18, arg19, arg20, arg21,	\
		arg22)

#define TRACE(lvl, name, tag)						\
  (_gpgex_debug (lvl, "%s (%s=0x%x): call\n",				\
		 name, STRINGIFY (tag), (void *)(uintptr_t)tag), 0)
#define TRACE0(lvl, name, tag, fmt)					\
  (_gpgex_debug (lvl, "%s (%s=0x%x): call: " fmt "\n",			\
		 name, STRINGIFY (tag), (void *) tag), 0)
#define TRACE1(lvl, name, tag, fmt, arg1)			       \
  (_gpgex_debug (lvl, "%s (%s=0x%x): call: " fmt "\n",		       \
		 name, STRINGIFY (tag), (void *) tag, arg1), 0)
#define TRACE2(lvl, name, tag, fmt, arg1, arg2)			       \
  (_gpgex_debug (lvl, "%s (%s=0x%x): call: " fmt "\n",		       \
		 name, STRINGIFY (tag), (void *) tag, arg1, arg2), 0)
#define TRACE3(lvl, name, tag, fmt, arg1, arg2, arg3)		       \
  (_gpgex_debug (lvl, "%s (%s=0x%x): call: " fmt "\n",		       \
		 name, STRINGIFY (tag), (void *) tag, arg1, arg2,      \
		 arg3), 0)
#define TRACE6(lvl, name, tag, fmt, arg1, arg2, arg3, arg4, arg5, arg6)	\
  (_gpgex_debug (lvl, "%s (%s=0x%x): call: " fmt "\n",			\
		 name, STRINGIFY (tag), (void *) tag, arg1, arg2, arg3,	\
		 arg4, arg5, arg6), 0)

#define TRACE_GPGERR(err)						\
  err == 0 ? (TRACE_SUC ()) :						\
    (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): error: %s <%s>\n", \
		   _gpgex_trace_func, _gpgex_trace_tagname,		\
		   _gpgex_trace_tag, gpg_strerror (err),		\
		   gpg_strsource (err)), (err))

#define TRACE_RES(err)							\
  err == S_OK ? (TRACE_SUC ()) :					\
    (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): %s: ec=%x\n",	\
		   _gpgex_trace_func, _gpgex_trace_tagname,		\
		   _gpgex_trace_tag,					\
		   SUCCEEDED (err) ? "leave" : "error",			\
		   err), (err))

#define TRACE_SUC()						   \
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): leave\n",	   \
		 _gpgex_trace_func, _gpgex_trace_tagname,	   \
		 _gpgex_trace_tag), 0)
#define TRACE_SUC0(fmt)							\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): leave: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag), 0)
#define TRACE_SUC1(fmt, arg1)						\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): leave: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1), 0)
#define TRACE_SUC2(fmt, arg1, arg2)					\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): leave: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2), 0)
#define TRACE_SUC5(fmt, arg1, arg2, arg3, arg4, arg5)			\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): leave: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5), 0)

#define TRACE_LOG(fmt)							\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag), 0)
#define TRACE_LOG1(fmt, arg1)						\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1), 0)
#define TRACE_LOG2(fmt, arg1, arg2)					\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2), 0)
#define TRACE_LOG3(fmt, arg1, arg2, arg3)				\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2, arg3), 0)
#define TRACE_LOG4(fmt, arg1, arg2, arg3, arg4)				\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2, arg3, arg4), 0)
#define TRACE_LOG6(fmt, arg1, arg2, arg3, arg4, arg5, arg6)		\
  (_gpgex_debug (_gpgex_trace_level, "%s (%s=0x%x): check: " fmt "\n",	\
		 _gpgex_trace_func, _gpgex_trace_tagname,		\
		 _gpgex_trace_tag, arg1, arg2, arg3, arg4, arg5,	\
		 arg6), 0)

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif
