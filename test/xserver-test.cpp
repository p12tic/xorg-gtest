#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <stdexcept>

#include <xorg/gtest/xorg-gtest.h>
#include <X11/extensions/XInput2.h>

using namespace xorg::testing;

TEST(XServer, LogRemoval)
{
  XORG_TESTCASE("X server startup and log file removal on success and error");
  std::string logfile = "/tmp/xorg-testing-xserver_____________.log";

  /* make sure a previous failed test didn't leave it around */
  unlink(logfile.c_str());

  XServer server;
  server.SetOption("-logfile", logfile);
  server.SetOption("-config", DUMMY_CONF_PATH);
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

TEST(XServer, WaitForSIGUSR1)
{
  XORG_TESTCASE("XOpenDisplay() following server.Start() must\n"
                "succeed as we wait for the SIGUSR1 signal\n");
  for (int i = 0; i < 20; i++) {
    XServer server;
    server.SetOption("-logfile", "/tmp/xorg-testing-xserver-sigusr1.log");
    server.SetOption("-config", DUMMY_CONF_PATH);
    server.SetOption("-noreset", "");
    server.Start();
    ASSERT_EQ(server.GetState(), Process::RUNNING);
    Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
    ASSERT_TRUE(dpy != NULL);
    XCloseDisplay(dpy);
    server.Terminate(500);
  }
}

static void assert_masks_equal(Display *dpy)
{
  int nmasks_before;
  XIEventMask *masks_before;
  int nmasks_after;
  XIEventMask *masks_after;

  masks_before = XIGetSelectedEvents(dpy, DefaultRootWindow(dpy), &nmasks_before);
  XServer::WaitForDevice(dpy, "not actually waiting for device", 1);
  masks_after = XIGetSelectedEvents(dpy, DefaultRootWindow(dpy), &nmasks_after);

  ASSERT_EQ(nmasks_before, nmasks_after);

  for (int i = 0; i < nmasks_before; i++) {
    ASSERT_EQ(masks_before[i].deviceid, masks_after[i].deviceid);
    ASSERT_EQ(masks_before[i].mask_len, masks_after[i].mask_len);
    ASSERT_EQ(memcmp(masks_before[i].mask, masks_after[i].mask, masks_before[i].mask_len), 0);
  }

  XFree(masks_before);
  XFree(masks_after);

}

TEST(XServer, WaitForDeviceEventMask)
{
  XORG_TESTCASE("The event mask is left as-is by WaitForDevice");

  XServer server;
  server.SetOption("-logfile", "/tmp/Xorg-WaitForDevice.log");
  server.SetOption("-config", DUMMY_CONF_PATH);
  server.SetOption("-noreset", "");
  server.Start();
  ASSERT_EQ(server.GetState(), Process::RUNNING);
  ::Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
  ASSERT_TRUE(dpy != NULL);
  int major = 2, minor = 0;
  XIQueryVersion(dpy, &major, &minor);

  /* empty mask */
  assert_masks_equal(dpy);

  /* device specific mask */
  XIEventMask m;
  m.deviceid = 2;
  m.mask_len = 1;
  m.mask = new unsigned char[m.mask_len]();
  XISetMask(m.mask, XI_Motion);
  XISelectEvents(dpy, DefaultRootWindow(dpy), &m, 1);

  assert_masks_equal(dpy);
  delete m.mask;

  /* XIAllDevices mask with short mask */
  m.deviceid = XIAllDevices;
  m.mask_len = 1;
  m.mask = new unsigned char[m.mask_len]();
  XISetMask(m.mask, XI_Motion);
  XISelectEvents(dpy, DefaultRootWindow(dpy), &m, 1);

  assert_masks_equal(dpy);
  delete m.mask;

  /* XIAllDevices mask with hierarchy bit not set */
  m.deviceid = XIAllDevices;
  m.mask_len = XIMaskLen(XI_HierarchyChanged);
  m.mask = new unsigned char[m.mask_len]();
  XISetMask(m.mask, XI_Motion);
  XISelectEvents(dpy, DefaultRootWindow(dpy), &m, 1);

  assert_masks_equal(dpy);
  delete m.mask;

  /* XIAllDevices mask with hierarchy bit set */
  m.deviceid = XIAllDevices;
  m.mask_len = XIMaskLen(XI_HierarchyChanged);
  m.mask = new unsigned char[m.mask_len]();
  XISetMask(m.mask, XI_HierarchyChanged);
  XISelectEvents(dpy, DefaultRootWindow(dpy), &m, 1);

  assert_masks_equal(dpy);
  delete m.mask;
}

#ifdef HAVE_EVEMU
TEST(XServer, WaitForExistingDevice)
{
  XORG_TESTCASE("WaitForDevice() returns true for already existing device");

  xorg::testing::evemu::Device d(TEST_ROOT_DIR "PIXART-USB-OPTICAL-MOUSE.desc");

  XServer server;
  server.SetOption("-logfile", "/tmp/Xorg-WaitForDevice.log");
  server.SetOption("-config", DUMMY_CONF_PATH);
  server.SetOption("-noreset", "");
  server.Start();
  ASSERT_EQ(server.GetState(), Process::RUNNING);
  ::Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
  ASSERT_TRUE(dpy != NULL);

  ASSERT_TRUE(XServer::WaitForDevice(dpy, "PIXART USB OPTICAL MOUSE", 1000));
}

TEST(XServer, WaitForNewDevice)
{
  XORG_TESTCASE("WaitForDevice() waits for newly created dvice");

  XServer server;
  server.SetOption("-logfile", "/tmp/Xorg-WaitForDevice.log");
  server.SetOption("-config", DUMMY_CONF_PATH);
  server.SetOption("-noreset", "");
  server.Start();
  ASSERT_EQ(server.GetState(), Process::RUNNING);
  ::Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
  ASSERT_TRUE(dpy != NULL);

  xorg::testing::evemu::Device d(TEST_ROOT_DIR "PIXART-USB-OPTICAL-MOUSE.desc");

  ASSERT_TRUE(XServer::WaitForDevice(dpy, "PIXART USB OPTICAL MOUSE", 1000));
}
#endif

TEST(XServer, IOErrorException)
{
  ASSERT_THROW({
  XServer server;
  server.SetOption("-logfile", "/tmp/xorg-io-error-test.log");
  server.SetOption("-config", DUMMY_CONF_PATH);
  server.SetOption("-noreset", "");
  server.Start();
  ASSERT_EQ(server.GetState(), Process::RUNNING);
  ::Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
  ASSERT_TRUE(dpy != NULL);
  close(ConnectionNumber(dpy));
  XSync(dpy, False);
  }, XIOError);
}

TEST(XServer, KeepAlive)
{
  XORG_TESTCASE("If XORG_GTEST_XSERVER_KEEPALIVE is set,\n"
                "XServer::Terminate() and XServer::Kill() have no "
                "effect");

  int pipefd[2];
  ASSERT_NE(pipe(pipefd), -1);

  if (fork() == 0) {
    close(pipefd[0]);

    ASSERT_EQ(setenv("XORG_GTEST_XSERVER_KEEPALIVE", "1", 1), 0);
    ASSERT_TRUE(getenv("XORG_GTEST_XSERVER_KEEPALIVE") != NULL);

    XServer server;
    server.SetOption("-logfile", "/tmp/Xorg-keepalive.log");
    server.SetOption("-config", DUMMY_CONF_PATH);
    server.SetOption("-noreset", "");
    server.Start();
    ASSERT_EQ(server.GetState(), Process::RUNNING);
    ::Display *dpy = XOpenDisplay(server.GetDisplayString().c_str());
    ASSERT_TRUE(dpy != NULL);

    server.Terminate();
    ASSERT_EQ(server.GetState(), Process::RUNNING);
    server.Kill();
    ASSERT_EQ(server.GetState(), Process::RUNNING);

    char *buffer;
    ASSERT_GT(asprintf(&buffer, "%d", server.Pid()), 0);
    ASSERT_EQ(write(pipefd[1], buffer, strlen(buffer)), (int)strlen(buffer));
    close(pipefd[1]);
    free(buffer);
    exit(0);
  }

  sigset_t sig_mask;
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGCHLD);
  struct timespec tv = { 1, 0 };
  sigprocmask(SIG_BLOCK, &sig_mask, NULL);

  /* parent */
  close(pipefd[1]);

  char buffer[20] = {0};
  ASSERT_GT(read(pipefd[0], buffer, sizeof(buffer)), 0);
  close(pipefd[0]);

  /* wait for forked child to die */
  ASSERT_EQ(sigtimedwait(&sig_mask, NULL, &tv), SIGCHLD);

  pid_t server_pid = atoi(buffer);

  /* server must still be running, kill it */
  ASSERT_EQ(kill(server_pid, 0), 0);
  kill(server_pid, SIGTERM);

  int i = 0;

  while(kill(server_pid, 0) == 0 && i++ < 10)
    usleep(50000);

  ASSERT_EQ(kill(server_pid, 0), -1);
  ASSERT_EQ(errno, ESRCH);
}

TEST(XServer, RemoveOption)
{
  XServer server;
  server.SetOption("-fail", "yes");
  server.SetOption("-logfile", "/tmp/Xorg-remove-option.log");
  server.Start(TEST_ROOT_DIR "/xserver-test-helper");
  ASSERT_EQ(server.GetState(), Process::FINISHED_FAILURE);

  server.RemoveOption("-fail");
  server.Start(TEST_ROOT_DIR "/xserver-test-helper");
  ASSERT_EQ(server.GetState(), Process::FINISHED_SUCCESS);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
