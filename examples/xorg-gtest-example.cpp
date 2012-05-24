#include <xorg/gtest/xorg-gtest.h>

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
