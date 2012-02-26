lib_LTLIBRARIES = libxorg-gtest.la libxorg-gtest_main.la

libxorg_gtest_la_SOURCES = \
	environment.cpp \
	process.cpp \
	test.cpp

libxorg_gtest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(GTEST_CPPFLAGS) \
	-DDUMMY_CONF_PATH="\"$(library_datadir)/dummy.conf\""

libxorg_gtest_main_la_SOURCES = \
	defines.h \
	main.cpp

libxorg_gtest_main_la_LIBADD = libxorg-gtest.la

library_includedir = $(includedir)/xorg/gtest
library_include_HEADERS = \
	$(top_srcdir)/include/xorg/gtest/environment.h \
	$(top_srcdir)/include/xorg/gtest/process.h \
	$(top_srcdir)/include/xorg/gtest/test.h
library_datadir = $(datadir)/xorg/gtest
library_data_DATA = $(top_srcdir)/conf/dummy.conf

libxorg_gtest_main_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(GTEST_CPPFLAGS)

libxorg_gtest_la_LDFLAGS = $(XSERVER_LIBS) -Wl,--version-script=libxorg-gtest.ver
libxorg_gtest_main_la_LDFLAGS = \
	$(XSERVER_LIBS) \
	-Wl,--version-script=libxorg-gtest_main.ver

AM_CXXFLAGS = -I$(top_srcdir)/include $(XSERVER_CFLAGS)

EXTRA_DIST = libxorg-gtest.ver libxorg-gtest_main.ver
