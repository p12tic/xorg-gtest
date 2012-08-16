#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>

#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>

using namespace xorg::testing;

TEST(XServer, LogRemoval)
{
  SCOPED_TRACE("TESTCASE: X server startup and log file removal on success and error");
  std::string logfile = "/tmp/xorg-testing-xserver_____________.log";

  /* make sure a previous failed test didn't leave it around */
  unlink(logfile.c_str());

  XServer server;
  server.SetOption("-logfile", logfile);
  server.Start();
  server.Terminate(3000);
  server.RemoveLogFile();

  std::ifstream file(logfile.c_str());
  ASSERT_FALSE(file.good());
  file.close();

  server.SetOption("-doesnotexist", "");
  server.Start();
  while (server.GetState() == Process::RUNNING)
    usleep(5000);

  ASSERT_EQ(server.GetState(), Process::FINISHED_FAILURE);
  file.open(logfile.c_str());
  ASSERT_FALSE(file.good()); /* server didn't leave the file behind */

  /* now create it */
  std::ofstream f(logfile.c_str());
  file.open(logfile.c_str());
  ASSERT_TRUE(file.good());
  file.close();

  /* must not remove it now */
  server.RemoveLogFile();

  file.open(logfile.c_str());
  ASSERT_TRUE(file.good()); /* server didn't remove it */
  file.close();

  server.RemoveLogFile(true);
  file.open(logfile.c_str());
  ASSERT_FALSE(file.good()); /* server did remove it */
  file.close();
}


int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
