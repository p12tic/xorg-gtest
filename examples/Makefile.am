#
# @file examples/Makefile.am 
# @brief automake recipe for the xorg-gtest examples
#
# Copyright 2011 Canonical, Ltd.
#
# This file is part of the utouch-geis library. This library is free software;
# you can redistribute it and/or modify it under the terms of the GNU Lesser
# General Public License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

noinst_PROGRAMS = xorg_gtest_example

xorg_gtest_example_SOURCES = xorg-gtest.cpp

xorg_gtest_example_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(GTEST_CPPFLAGS)

xorg_gtest_example_LDFLAGS = -lxorg-gtest -lxorg-gtest_main -lgtest -lpthread -lX11
