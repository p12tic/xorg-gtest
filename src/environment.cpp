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

#include "xorg/gtest/environment.h"
#include "xorg/gtest/process.h"
#include "defines.h"

#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
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
  d_->path_to_log_file = path_to_log_file;
}

const std::string& xorg::testing::Environment::log_file() const
{
  return d_->path_to_log_file;
}

void xorg::testing::Environment::set_conf_file(const std::string& path_conf_file)
{
  d_->path_to_conf = path_conf_file;
}

const std::string& xorg::testing::Environment::conf_file() const
{
  return d_->path_to_conf;
}

void xorg::testing::Environment::set_server(const std::string& path_to_server)
{
  d_->path_to_server = path_to_server;
}

const std::string& xorg::testing::Environment::server() const
{
  return d_->path_to_server;
}

void xorg::testing::Environment::set_display(int display_num)
{
  d_->display = display_num;
}

int xorg::testing::Environment::display() const
{
  return d_->display;
}

void xorg::testing::Environment::SetUp() {
  static char display_string[6];
  snprintf(display_string, 6, ":%d", d_->display);

  d_->process.Start(d_->path_to_server, d_->path_to_server.c_str(),
                    display_string,
                    "-logfile", d_->path_to_log_file.c_str(),
                    "-config", d_->path_to_conf.c_str(),
                    NULL);

  Process::SetEnv("DISPLAY", display_string, true);

  for (int i = 0; i < 10; ++i) {
    Display* display = XOpenDisplay(NULL);

    if (display) {
      XCloseDisplay(display);
      return;
    }

    int status;
    int pid = waitpid(d_->process.Pid(), &status, WNOHANG);
    if (pid == d_->process.Pid())
      throw std::runtime_error("Dummy X server failed to start, did you run as "
                               "root?");
    else if (pid == 0)
      sleep(1); /* Give the dummy X server some time to start */
    else if (pid == -1)
      throw std::runtime_error("Could not get status of dummy X server "
                               "process");
    else
      throw std::runtime_error("Invalid child PID returned by Process::Wait()");
  }

  throw std::runtime_error("Unable to open connection to dummy X server");
}

void xorg::testing::Environment::TearDown() {
  if (!d_->process.Terminate()) {
    std::cerr << "Warning: Failed to terminate dummy Xorg server: "
              << std::strerror(errno) << "\n";
    if (!d_->process.Kill())
      std::cerr << "Warning: Failed to kill dummy Xorg server: "
                << std::strerror(errno) << "\n";
  }
}
