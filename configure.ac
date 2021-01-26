# configure.ac - for GpgEX
# Copyright (C) 2005, 2006, 2007, 2013 g10 Code GmbH
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)
min_automake_version="1.10"

# To build a release you need to create a tag with the version number
# (git tag -s gnupg-2.n.m) and run "./autogen.sh --force".  Please
# bump the version number immediately *after* the release and do
# another commit and push so that the git magic is able to work.
m4_define([mym4_version], [1.0.8])

# Below is m4 magic to extract and compute the git revision number,
# the decimalized short revision number, a beta version string and a
# flag indicating a development version (mym4_isgit).  Note that the
# m4 processing is done by autoconf and not during the configure run.
m4_define([mym4_revision],
          m4_esyscmd([git rev-parse --short HEAD | tr -d '\n\r']))
m4_define([mym4_revision_dec],
          m4_esyscmd_s([echo $((0x$(echo ]mym4_revision[|head -c 4)))]))
m4_define([mym4_betastring],
          m4_esyscmd_s([git describe --match 'gpgex-[0-9].[0-9].*[0-9]' --long|\
                        awk -F- '$3!=0{print"-beta"$3}']))
m4_define([mym4_isgit],m4_if(mym4_betastring,[],[no],[yes]))
m4_define([mym4_full_version],[mym4_version[]mym4_betastring])

AC_INIT([gpgex],[mym4_full_version], [http://bugs.gnupg.org])

NEED_GPG_ERROR_VERSION=1.12

NEED_LIBASSUAN_API=2
NEED_LIBASSUAN_VERSION=2.1.1

PACKAGE=$PACKAGE_NAME
PACKAGE_GT=${PACKAGE_NAME}
VERSION=$PACKAGE_VERSION

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(src/gpgex.def)
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

AC_GNU_SOURCE

have_gpg_error=no
have_libassuan=no

AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_GT)
AC_SUBST(VERSION)
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of this package])
AC_DEFINE_UNQUOTED(PACKAGE_GT, "$PACKAGE_GT",
                                [Name of this package for gettext])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version of this package])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "$PACKAGE_BUGREPORT",
                                        [Bug report address])


AH_BOTTOM([
/* Some global constants. */

/* Force using of NLS for W32 even if no libintl has been found.  This is
   okay because we have our own gettext implementation for W32.  */
#if defined(HAVE_W32_SYSTEM) && !defined(ENABLE_NLS)
#define ENABLE_NLS 1
#endif
])

AM_MAINTAINER_MODE
AM_SILENT_RULES

# Checks for programs.
AC_PROG_MAKE_SET
AM_SANITY_CHECK
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar, :)
AC_CHECK_TOOL(DLLTOOL, dlltool, :)
AC_CHECK_TOOL(WINDRES, windres, :)


have_dosish_system=no
have_w32_system=no
have_w64_system=no
case "${host}" in
    x86_64-*mingw32*)
        have_dosish_system=yes
        have_w32_system=yes
        have_w64_system=yes
        BUILD_CPU_ARCH=amd64
        ;;
    *-mingw32*)
        have_dosish_system=yes
        have_w32_system=yes
        BUILD_CPU_ARCH=x86
        ;;
    *)
    AC_MSG_ERROR([[
***
*** This software my only be build for W32 systems.  Use
***     ./autogen.sh --build-w32
*** or
***     ./autogen.sh --build-w64
*** to prepare it for such a build.
***]])
       ;;
esac

# We need the CPU architecture for the manifest file.
AC_SUBST(BUILD_CPU_ARCH)


AC_DEFINE(HAVE_DRIVE_LETTERS,1,
          [defined if we must run on a stupid file system])

if test "$have_dosish_system" = yes; then
   AC_DEFINE(HAVE_DOSISH_SYSTEM,1,
             [Defined if we run on some of the PCDOS like systems
              (DOS, Windoze. OS/2) with special properties like
              no file modes])
fi
AM_CONDITIONAL(HAVE_DOSISH_SYSTEM, test "$have_dosish_system" = yes)

if test "$have_w32_system" = yes; then
   AC_DEFINE(HAVE_W32_SYSTEM,1, [Defined if we run on a W32 API based system])
   if test "$have_w64_system" = yes; then
      AC_DEFINE(HAVE_W64_SYSTEM,1,
                [Defined if we run on a 64 bit W32 API based system])
   fi
fi
AM_CONDITIONAL(HAVE_W32_SYSTEM, test "$have_w32_system" = yes)
AM_CONDITIONAL(HAVE_W64_SYSTEM, test "$have_w64_system" = yes)

gpa_only=no
AC_ARG_ENABLE(gpa-only,
              AC_HELP_STRING([--enable-gpa-only],
                             [Try only GPA as UI-server]),
              gpa_only=$enableval)
AC_MSG_RESULT($gpa_only)
if test "$gpa_only" = yes ; then
    AC_DEFINE(ENABLE_GPA_ONLY, 1,
             [Define to use only GPA as UI-server])
fi


#
# Checks for libraries.
#

# Note, that autogen.sh greps for the next line.
AM_GNU_GETTEXT_VERSION([0.12.1])
AM_GNU_GETTEXT([external])
# We always want NLs, despite what the above macro figures out.
USE_NLS=yes

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h unistd.h langinfo.h termio.h locale.h)

AC_CHECK_FUNCS(stpcpy)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

# Add some extra libs here so that previous tests don't fail for
# mysterious reasons - the final link step should bail out.
if test "$have_w32_system" = yes; then
   W32LIBS="-lwsock32"
fi

if test "$GCC" = yes; then
    CFLAGS="$CFLAGS -Wall -mms-bitfields"
    CXXFLAGS="$CXXFLAGS -Wall -mms-bitfields"
    if test "$USE_MAINTAINER_MODE" = "yes"; then
        CFLAGS="$CFLAGS -Wcast-align -Wshadow -Wstrict-prototypes"
        CFLAGS="$CFLAGS -Wno-format-y2k -Wformat-security"
        CXXFLAGS="$CXXFLAGS -Wcast-align -Wshadow"
        CXXFLAGS="$CXXFLAGS -Wno-format-y2k -Wformat-security"
    fi
    HARDENING="-Wl,--dynamicbase -Wl,--nxcompat -fno-exceptions -D_FORTIFY_SOURCE=2 -O0"
    CFLAGS="$CFLAGS $HARDENING"
    CXXFLAGS="$CXXFLAGS $HARDENING"
fi

AC_SUBST(W32LIBS)


#
# libgpg-error is a library with error codes shared between GnuPG
# related projects.
#
AM_PATH_GPG_ERROR("$NEED_GPG_ERROR_VERSION",
                  have_gpg_error=yes,have_gpg_error=no)
AC_DEFINE(GPG_ERR_SOURCE_DEFAULT, GPG_ERR_SOURCE_USER_1,
          [The default error source for GpgEX.])

# We need the declaration for the function GetUserDefaultUILanguage
# and AllowSetForegroundWindow.
AC_DEFINE(WINVER, 0x500, [Version of Windows API])

#
# libassuan is used for IPC
#
AM_PATH_LIBASSUAN("$NEED_LIBASSUAN_API:$NEED_LIBASSUAN_VERSION",
                  have_libassuan=yes,have_libassuan=no)


AH_BOTTOM([
/* We don't want the old assuan codes anymore. */
#define _ASSUAN_ONLY_GPG_ERRORS 1
/* We use the gettext code from libgpg-error.  */
#define GPG_ERR_ENABLE_GETTEXT_MACROS

])


#
# Provide information about the build.
#
BUILD_REVISION="mym4_revision"
AC_SUBST(BUILD_REVISION)
AC_DEFINE_UNQUOTED(BUILD_REVISION, "$BUILD_REVISION",
                   [GIT commit id revision used to build this package])

changequote(,)dnl
BUILD_FILEVERSION=`echo "$VERSION" | sed 's/\([0-9.]*\).*/\1./;s/\./,/g'`
changequote([,])dnl
BUILD_FILEVERSION="${BUILD_FILEVERSION}mym4_revision_dec"
AC_SUBST(BUILD_FILEVERSION)
BUILD_FILEVERSION_DOT=`echo "$BUILD_FILEVERSION" | tr ',' '.'`
AC_SUBST(BUILD_FILEVERSION_DOT)

BUILD_TIMESTAMP=`date -u +%Y-%m-%dT%H:%M+0000 2>/dev/null || date`
AC_SUBST(BUILD_TIMESTAMP)
AC_DEFINE_UNQUOTED(BUILD_TIMESTAMP, "$BUILD_TIMESTAMP",
                   [The time this package was configured for a build])
BUILD_HOSTNAME="$ac_hostname"
AC_SUBST(BUILD_HOSTNAME)


#
# Print errors here so that they are visible all
# together and the user can acquire them all together.
#
die=no
if test "$have_gpg_error" = "no"; then
   die=yes
   AC_MSG_NOTICE([[
***
*** You need libgpg-error to build this program.
**  This library is for example available at
***   ftp://ftp.gnupg.org/gcrypt/libgpg-error
*** (at least version $NEED_GPG_ERROR_VERSION is required.)
***]])
fi
if test "$have_libassuan" = "no"; then
   die=yes
   AC_MSG_NOTICE([[
***
*** You need libassuan to build this program.
*** This library is for example available at
***   ftp://ftp.gnupg.org/gcrypt/libassuan/
*** (at least version $NEED_LIBASSUAN_VERSION is required).
***]])
fi
if test "$die" = "yes"; then
    AC_MSG_ERROR([[
***
*** Required libraries not found. Please consult the above messages
*** and install them before running configure again.
***]])
fi


AC_CONFIG_FILES([ Makefile
doc/Makefile
src/Makefile
src/versioninfo.rc
src/gpgex.manifest
po/Makefile.in
m4/Makefile
])
AC_OUTPUT
