SUBDIRS = src doc examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xorg-gtest.pc

ACLOCAL_AMFLAGS = -I m4

.PHONY: ChangeLog INSTALL
INSTALL:
	$(INSTALL_CMD)

.PHONY: doc
doc:
	$(MAKE) -C doc $@

dist-hook: ChangeLog INSTALL

EXTRA_DIST = conf
