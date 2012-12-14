#include <xorg/gtest/xorg-gtest.h>

#include <sstream>

#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>

using namespace xorg::testing;

/**
 * @example xorg-gtest-example.cpp
 *
 * These are examples for using xorg-gtest in tests and test fixtures.
 *
 * Please make sure that you have the X.org dummy display driver installed
 * on your system and that you execute the test with root privileges.
 *
 * Each of these tests starts a new server.
 */

/**
 * Basic test, starts the server, checks it is running, then terminates it.
 */
TEST(XServer, StartServer) {
  XServer server;
  server.SetOption("-logfile", LOGFILE_DIR "/xserver-startserver.log");
  server.Start();

  ASSERT_EQ(server.GetState(), Process::RUNNING);
  ASSERT_TRUE(server.Terminate());
  ASSERT_EQ(server.GetState(), Process::FINISHED_SUCCESS);

  /* If we get here, we were successful,so remove the log file */
  server.RemoveLogFile();
}

/**
 * Start a server, check the display connection works, terminate the server
 */
TEST(XServer, DisplayConnection) {
  XServer server;
  server.SetOption("-logfile", LOGFILE_DIR "/xserver-display-connection.log");
  server.Start();

  Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
  ASSERT_TRUE(dpy != NULL);

  /* calling Terminate isn't necessary as the destructor will do it for us,
     but we do want the log file removed. That only works on terminated
     servers. */
  server.Terminate();
  server.RemoveLogFile();
}

/**
 * Example for a test fixture that starts a server per test.
 */
class ServerTest : public Test {
public:
  virtual void SetUp(void) {
    const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();

    /* Use a log file based on the test name. NOTE: test names for
       parameterized tests end in /0, /1, etc. */
    std::stringstream log;
    log << LOGFILE_DIR "/xserver-";
    log << test_info->test_case_name() << "." << test_info->name();
    log << ".log";

    server.SetOption("-logfile", log.str());
    server.Start();

    /* set up Display() */
    Test::SetDisplayString(server.GetDisplayString());
    ASSERT_NO_FATAL_FAILURE(Test::SetUp());
  }

  virtual void TearDown(void) {
    ASSERT_TRUE(server.Terminate());
    server.RemoveLogFile();
  }
protected:
  /** The X server instance */
  XServer server;
};

/**
 * Two mostly pointless tests. Both use the same test fixture class
 * above. Note that the server is restarted after each test.
 */
TEST_F(ServerTest, DisplayWidth) {
  ASSERT_GT(DisplayWidth(Display(), 0), 0);
}

TEST_F(ServerTest, DisplayHeight) {
  ASSERT_GT(DisplayHeight(Display(), 0), 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
