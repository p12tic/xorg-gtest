#
# @file examples/Makefile.am
# @brief automake recipe for the xorg-gtest self-tests
#
# Copyright (C) 2011, 2012 Canonical, Ltd.
# Copyright © 2012 Red Hat, Inc.
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

noinst_PROGRAMS = process-test xserver-test device-test
noinst_DATA = PIXART-USB-OPTICAL-MOUSE.desc

AM_CPPFLAGS = $(GTEST_CPPFLAGS)
AM_CXXFLAGS = $(BASE_CXXFLAGS) \
	      -DTEST_ROOT_DIR=\"$(abs_top_srcdir)/test/\"

tests_libraries = \
	libgtest.a \
	libxorg-gtest.a \
	-lpthread \
	$(X11_LIBS) \
	$(EVEMU_LIBS)

process_test_SOURCES = process-test.cpp
process_test_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/include
process_test_LDADD =  $(tests_libraries)

xserver_test_SOURCES = xserver-test.cpp
xserver_test_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/include
xserver_test_LDADD =  $(tests_libraries)

device_test_SOURCES = device-test.cpp
device_test_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/include
device_test_LDADD =  $(tests_libraries)

check_LIBRARIES = libgtest.a libxorg-gtest.a

# build googletest as static lib
nodist_libgtest_a_SOURCES = $(GTEST_SOURCE)/src/gtest-all.cc
libgtest_a_CPPFLAGS = $(AM_CPPFLAGS) -w
libgtest_a_CXXFLAGS = $(GTEST_CXXFLAGS) $(AM_CXXFLAGS)

# build xorg-gtest as static lib
libxorg_gtest_a_SOURCES = $(top_srcdir)/src/xorg-gtest-all.cpp
libxorg_gtest_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir) \
	-DDUMMY_CONF_PATH="\"$(top_srcdir)/data/xorg/gtest/dummy.conf\""
libxorg_gtest_a_CXXFLAGS = $(GTEST_CXXFLAGS) $(AM_CXXFLAGS)

if ENABLE_XORG_GTEST_TESTS
TESTS = $(noinst_PROGRAMS)
endif
