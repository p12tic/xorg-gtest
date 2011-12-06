lib_LTLIBRARIES = libxorg-gtest.la libxorg-gtest_main.la

libxorg_gtest_la_SOURCES = \
	src/environment.cpp

libxorg_gtest_main_la_SOURCES = \
	src/main.cpp

library_includedir=$(includedir)/xorg/gtest
library_include_HEADERS = include/xorg/gtest/environment.h

library_datadir=$(datadir)/xorg/gtest
library_data_DATA=conf/dummy.conf

libxorg_gtest_main_la_CPPFLAGS = $(AM_CPPFLAGS) -DDUMMY_CONF_PATH="\"$(library_datadir)/dummy.conf\""

libxorg_gtest_la_LDFLAGS = $(X11_LIBS)
libxorg_gtest_main_la_LDFLAGS = $(X11_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xorg-gtest.pc

ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = -Iinclude

.PHONY: ChangeLog INSTALL
INSTALL:
	$(INSTALL_CMD)

doc-%:
	$(MAKE) -C doc $@

dist-hook: ChangeLog INSTALL
