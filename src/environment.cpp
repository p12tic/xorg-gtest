/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011, 2012 Canonical Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ******************************************************************************/

#include "xorg/gtest/xorg-gtest-environment.h"
#include "xorg/gtest/xorg-gtest-process.h"
#include "defines.h"

#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <X11/Xlib.h>

struct xorg::testing::Environment::Private {
  Private()
      : path_to_conf(DUMMY_CONF_PATH), path_to_log_file(DEFAULT_XORG_LOGFILE),
        path_to_server(DEFAULT_XORG_SERVER), display(DEFAULT_DISPLAY) {
  }

  std::string path_to_conf;
  std::string path_to_log_file;
  std::string path_to_server;
  int display;
  Process process;
};

xorg::testing::Environment::Environment()
    : d_(new Private) {
}

xorg::testing::Environment::~Environment() {}

void xorg::testing::Environment::set_log_file(const std::string& path_to_log_file)
{
  SetLogFile(path_to_log_file);
}

void xorg::testing::Environment::SetLogFile(const std::string& path_to_log_file)
{
  d_->path_to_log_file = path_to_log_file;
}

const std::string& xorg::testing::Environment::GetLogFile() const
{
  return d_->path_to_log_file;
}

void xorg::testing::Environment::SetConfigFile(const std::string& path_to_conf_file)
{
  d_->path_to_conf = path_to_conf_file;
}

const std::string& xorg::testing::Environment::GetConfigFile() const
{
  return d_->path_to_conf;
}

void xorg::testing::Environment::SetServerPath(const std::string& path_to_server)
{
  d_->path_to_server = path_to_server;
}

const std::string& xorg::testing::Environment::GetServerPath() const
{
  return d_->path_to_server;
}

void xorg::testing::Environment::SetDisplayNumber(int display_num)
{
  d_->display = display_num;
}

void xorg::testing::Environment::SetUp() {
  static char display_string[6];
  snprintf(display_string, 6, ":%d", d_->display);

  Display* test_display = XOpenDisplay(display_string);
  if (test_display) {
    XCloseDisplay(test_display);
    std::string message;
    message += "A server is already running on ";
    message += display_string;
    message += ".";
    throw std::runtime_error(message);
  }

  /* The Xorg server won't start unless the log file and the old log file are
   * writable. */
  std::ofstream log_test;
  log_test.open(d_->path_to_log_file.c_str(), std::ofstream::out);
  log_test.close();
  if (log_test.fail()) {
    std::string message;
    message += "X.org server log file ";
    message += d_->path_to_log_file;
    message += " is not writable.";
    throw std::runtime_error(message);
  }

  std::string old_log_file = d_->path_to_log_file.c_str();
  old_log_file += ".old";
  log_test.open(old_log_file.c_str(), std::ofstream::out);
  log_test.close();
  if (log_test.fail()) {
    std::string message;
    message += "X.org old server log file ";
    message += old_log_file;
    message += " is not writable.";
    throw std::runtime_error(message);
  }

  d_->process.Start(d_->path_to_server, d_->path_to_server.c_str(),
                    display_string,
                    "-logverbose", "10",
                    "-logfile", d_->path_to_log_file.c_str(),
                    "-config", d_->path_to_conf.c_str(),
                    NULL);

  Process::SetEnv("DISPLAY", display_string, true);

  for (int i = 0; i < 10; ++i) {
    test_display = XOpenDisplay(NULL);

    if (test_display) {
      XCloseDisplay(test_display);
      return;
    }

    int status;
    int pid = waitpid(d_->process.Pid(), &status, WNOHANG);
    if (pid == d_->process.Pid()) {
      std::string message;
      message += "X server failed to start on display ";
      message += display_string;
      message += ". Ensure that the \"dummy\" video driver is installed.\n"
                 "If the X.org server is older than 1.12, "
                 "tests will need to be run as root.\nCheck ";
      message += d_->path_to_log_file;
      message += " for any errors";
      throw std::runtime_error(message);
    } else if (pid == 0) {
      sleep(1); /* Give the dummy X server some time to start */
    } else if (pid == -1) {
      throw std::runtime_error("Could not get status of dummy X server "
                               "process");
    } else {
      throw std::runtime_error("Invalid child PID returned by Process::Wait()");
    }
  }

  throw std::runtime_error("Unable to open connection to dummy X server");
}

void xorg::testing::Environment::TearDown() {
  if (d_->process.Terminate()) {
    for (int i = 0; i < 10; i++) {
      int status;
      int pid = waitpid(d_->process.Pid(), &status, WNOHANG);

      if (pid == d_->process.Pid())
        return;

      sleep(1); /* Give the dummy X server more time to shut down */
    }
  }

  Kill();
}

void xorg::testing::Environment::Kill() {
  if (!d_->process.Kill())
    std::cerr << "Warning: Failed to kill dummy Xorg server: "
              << std::strerror(errno) << "\n";

  for (int i = 0; i < 10; i++) {
    int status;
    int pid = waitpid(d_->process.Pid(), &status, WNOHANG);

    if (pid == d_->process.Pid())
      return;

      sleep(1); /* Give the dummy X server more time to shut down */
  }

  std::cerr << "Warning: Dummy X server did not shut down\n";
}


/* DEPRECATED */
const std::string& xorg::testing::Environment::log_file() const
{
  return GetLogFile();
}

void xorg::testing::Environment::set_conf_file(const std::string& path_conf_file)
{
  return SetConfigFile(path_conf_file);
}

const std::string& xorg::testing::Environment::conf_file() const
{
  return GetConfigFile();
}

void xorg::testing::Environment::set_server(const std::string& path_to_server)
{
  SetServerPath(path_to_server);
}

const std::string& xorg::testing::Environment::server() const
{
  return GetServerPath();
}

void xorg::testing::Environment::set_display(int display_num)
{
  SetDisplayNumber(display_num);
}

int xorg::testing::Environment::display() const
{
  return d_->display;
}
