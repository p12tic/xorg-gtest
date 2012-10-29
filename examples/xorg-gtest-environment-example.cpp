#include <xorg/gtest/xorg-gtest.h>

#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>

using namespace xorg::testing;

/**
 * @example xorg-gtest-environment-example.cpp
 *
 * This is an example for using the fixture xorg::testing::Test for your own
 * tests, using the xorg::testing::Environment to start up a server.
 *
 * Please make sure that you have the X.org dummy display driver installed
 * on your system and that you execute the test with root privileges.
 *
 * The xorg::testing::Environment starts up one server before the test
 * execution, that server is alive during all tests until the end of the
 * test suite. For options on controlling that server run this test with
 * --help, the configuration path, display number can be controlled by the
 *  caller.
 *
 * Note that the test environment (i.e. the X server instance) is shared
 * between tests. If the tests are the only client connection, the server
 * will trigger a regeneration when the test finishes with Test::TearDown().
 * If more than one client connection is active however, this regeneration
 * will not happen and changes made by one test can affect outcomes on other
 * tests.
 *
 * This test is missing a main(), we pull that in from xorg-gtest-main.cpp
 */

/* This must be a TEST_F to ensure we get the goodies from
 * ::xorg::testing::Test */
TEST_F(Test, DummyXorgServerTest) {

  /* Display() contains the display connection to the X server */
  EXPECT_NE((Window)None, DefaultRootWindow(Display()));

}

/* Another test querying for input devices. */
TEST_F(Test, XIQueryVersion20) {
  int major = 2, minor = 0;

  ASSERT_EQ(Success, XIQueryVersion(Display(), &major, &minor));
  ASSERT_GE(major * 100 + minor, 200);
}

/* Even though we queried for 2.0 above, we can query again for 2.2. Each
 * TEST_F has a new Display */
TEST_F(Test, XIQueryVersion22) {
  int major = 2, minor = 2;

  ASSERT_EQ(Success, XIQueryVersion(Display(), &major, &minor));
  ASSERT_EQ(major * 100 + minor, 202);
}

/* The next two tests illustrates a potential test dependency.
 * Test.CreateWindowProperty creates a property on the root window. If our
 * connection is the only Display connection, the server will regenerate
 * after Test::TearDown and the property is removed becore
 * Test.CheckWindowProperty is called. If some other client holds the
 * connection open, the property stays alive.
 */
TEST_F(Test, CreateWindowProperty) {
  Atom prop = XInternAtom(Display(), "xorg-gtest test atom", False);
  ASSERT_NE((Atom)None, prop);

  unsigned char data = 1;
  XChangeProperty(Display(), DefaultRootWindow(Display()), prop,
                  XA_INTEGER, 8, PropModeReplace, &data, 1);
  XFlush(Display());
}

TEST_F(Test, CheckWindowProperty) {
  Atom prop = XInternAtom(Display(), "xorg-gtest test atom", True);
  ASSERT_EQ((Atom)None, prop) << "Property did not get removed, some client prevented regeneration";
}

/**
 * Example class for how to subclass tests.
 */
class SubTest : public Test {
public:
  virtual void SetUp(void) {
    Test::SetUp();

    /* Create some atom that we can then use inside the test */
    example_prop = XInternAtom(Display(), "xorg-gtest example atom", False);
  }

protected:
  Atom example_prop;
};

TEST_F(SubTest, ExampleAtom) {
  /* We have access to SubTest's member public and protected variables here */
  ASSERT_NE((Atom)None, example_prop);
}
