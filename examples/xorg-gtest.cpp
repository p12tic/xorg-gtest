/*****************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include <xorg/gtest/test.h>

using namespace xorg::testing;

/**
 * @example xorg-gtest.cpp
 *
 * This is an example for using the fixture
 * xorg::testing::Test for your own tests. Please
 * make sure that you have the X.org dummy display
 * driver installed on your system and that you execute
 * the test with root privileges.
 */
TEST_F(Test, DummyXorgServerTest) {

  EXPECT_NE(0, DefaultRootWindow(Display()));

}
