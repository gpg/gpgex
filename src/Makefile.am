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

AM_CFLAGS = $(LIBASSUAN_CFLAGS) $(GPG_ERROR_CFLAGS) -shared
AM_CXXFLAGS = $(LIBASSUAN_CFLAGS) $(GPG_ERROR_CFLAGS) -shared

ICON_NAMES = key
ICON_SIZES = 12 16
ICONS = $(addprefix icon-, \
	$(foreach size,$(ICON_SIZES),$(addsuffix -$(size).bmp, $(ICON_NAMES))))

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
	client.h client.cc			\
	main.h main.cc				\
	$(ICONS)

libgpg-error.a:
	ln -sf $(shell $(GPG_ERROR_CONFIG) --prefix)/lib/libgpg-error.a

libassuan.a:
	ln -sf $(shell $(LIBASSUAN_CONFIG) --prefix)/lib/libassuan.a

clean-local:
	rm -f libgpg-error.a libassuan.a

#gpgex_LDADD = $(srcdir)/gpgex.def		\
#	-L . -lshell32  -lcomdlg32 -ladvapi32

# We need -loleaut32 for start_help() in gpgex.cc.
gpgex_LDADD = $(srcdir)/gpgex.def -L . \
	-lshell32 -lgdi32 -lole32 -luuid \
	./libassuan.a ./libgpg-error.a -lws2_32 -loleaut32

.rc.o:
	$(WINDRES) -I $(srcdir) -I . `test -f '$<' || echo '$(srcdir)/'`$< $@
