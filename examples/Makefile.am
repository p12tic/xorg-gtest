#
# @file examples/Makefile.am 
# @brief automake recipe for the xorg-gtest examples
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

check_LIBRARIES = libgtest.a libxorg-gtest.a libxorg-gtest_main.a

GTEST_CPPFLAGS=-I$(top_srcdir)/gtest/include -I$(top_srcdir)/gtest

AM_CPPFLAGS = $(GTEST_CPPFLAGS)
AM_CXXFLAGS = $(X11_CFLAGS) $(BASE_CXXFLAGS)

# build googletest as static lib
nodist_libgtest_a_SOURCES = $(top_srcdir)/gtest/src/gtest-all.cc
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

# build xorg-gtest's main as separate lib for those that need it
libxorg_gtest_main_a_SOURCES = $(top_srcdir)/src/xorg-gtest_main.cpp
libxorg_gtest_main_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)
libxorg_gtest_main_a_CXXFLAGS = $(GTEST_CXXFLAGS) $(AM_CXXFLAGS)

noinst_PROGRAMS = xorg-gtest-example

if ENABLE_XORG_GTEST_TESTS
TESTS = $(noinst_PROGRAMS)
endif

xorg_gtest_example_SOURCES = xorg-gtest-example.cpp

xorg_gtest_example_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/include

xorg_gtest_example_LDADD = \
	libgtest.a \
	libxorg-gtest.a \
	libxorg-gtest_main.a \
	-lpthread \
	$(X11_LIBS) \
	$(EVEMU_LIBS)
