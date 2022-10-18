# Makefile.am - main makefile for GpgEX
# Copyright (C) 2005 g10 Code GmbH
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = no-dist-gzip dist-bzip2
# Because we can only build the w32 version we need to help automake.
DISTCHECK_CONFIGURE_FLAGS = --host=@host@ --build=@build@ \
                            --prefix=@prefix@

EXTRA_DIST = autogen.sh autogen.rc


SUBDIRS = doc src po m4

dist-hook:
	echo "$(VERSION)" > $(distdir)/VERSION

distcheck-hook:
	set -e; ( \
	pref="#+macro: gpgex_" ;\
	reldate="$$(date -u +%Y-%m-%d)" ;\
        echo "$${pref}ver  $(PACKAGE_VERSION)"  ;\
        echo "$${pref}date $${reldate}" ;\
        list='$(DIST_ARCHIVES)'; for i in $$list; do \
	  case "$$i" in *.tar.bz2) \
            echo "$${pref}size $$(wc -c <$$i|awk '{print int($$1/1024)}')k" ;\
	    echo "$${pref}sha1 $$(sha1sum <$$i|cut -d' ' -f1)" ;\
	    echo "$${pref}sha2 $$(sha256sum <$$i|cut -d' ' -f1)" ;;\
	  esac;\
	done ) | tee $(distdir).swdb
