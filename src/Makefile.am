#
# Makefile for the src subdirectory of xorg-gtest
#
# Copyright (C) 2011, 2012 Canonical, Ltd.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

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

libxorg_gtest_la_LDFLAGS = $(XSERVER_LIBS) -Wl,--version-script=$(top_srcdir)/src/libxorg-gtest.ver
libxorg_gtest_main_la_LDFLAGS = \
	$(XSERVER_LIBS) \
	-Wl,--version-script=$(top_srcdir)/src/libxorg-gtest_main.ver

AM_CXXFLAGS = -I$(top_srcdir)/include $(XSERVER_CFLAGS)

EXTRA_DIST = libxorg-gtest.ver libxorg-gtest_main.ver
