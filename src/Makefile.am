# Makefile.am - main makefile for dialogs part of GpgEX
# Copyright (C) 2005, 2007 g10 Code GmbH
# 
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = gpgex
EXTRA_DIST = versioninfo.rc.in
EXEEXT = .dll

AM_CFLAGS = $(GPGME_CFLAGS) -shared
AM_CXXFLAGS = $(GPGME_CFLAGS) -shared

nodist_gpgex_SOURCES = versioninfo.rc
gpgex_SOURCES = 				\
	gpgex.def				\
	icons.rc 				\
	registry.h registry.c			\
	w32-gettext.c w32-gettext.h		\
	gpgex-class.h gpgex-class.cc		\
	gpgex-factory.h gpgex-factory.cc	\
	bitmaps.h bitmaps.cc			\
	gpgex.h gpgex.cc			\
	main.h main.cc

#gpgex_LDADD = $(srcdir)/gpgex.def		\
#	-L . -lshell32  -lcomdlg32 -loleaut32 -ladvapi32
gpgex_LDADD = $(srcdir)/gpgex.def -L . \
	-lshell32 -lgdi32 -lws2_32 -lole32 -luuid

.rc.o:
	$(WINDRES) -I $(srcdir) -I . `test -f '$<' || echo '$(srcdir)/'`$< $@
