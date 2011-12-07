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

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <X11/Xlib.h>

namespace xorg {
namespace testing {
struct Environment::Private {
  Private(const std::string& path, int display)
      : path_to_conf_(path),
        display_(display) {
  }

  std::string path_to_conf_;
  int display_;
  Process process_;
};
} // namespace testing
} // namespace xorg

xorg::testing::Environment::Environment(const std::string& path, int display)
    : d_(new Private(path, display)) {

}

xorg::testing::Environment::~Environment() {
  delete d_;
}

void xorg::testing::Environment::SetUp() {
  static char display_string[6];
  snprintf(display_string, 6, ":%d", d_->display_);

  if (d_->process_.Start("Xorg", display_string, d_->path_to_conf_.c_str())) {
    setenv("DISPLAY", display_string, true);

    for (int i = 0; i < 10; /*++i*/) {
      Display* display = XOpenDisplay(NULL);

      if (display) {
        XCloseDisplay(display);
        return;
      }

      int status;
      int pid = d_->process_.Wait(&status, WNOHANG); //waitpid(d_->child_pid_, &status, WNOHANG);
      if (pid == d_->process_.pid()) {
        // d_->child_pid_ = -1;
        FAIL() << "Dummy X server failed to start, did you run as root?";
        return;
      } else if (pid == 0) {
        sleep(1); /* Give the dummy X server some time to start */
        continue;
      } else if (pid == -1) {
        FAIL() << "Could not get status of dummy X server process: "
            << std::strerror(errno);
        return;
      } else {
        FAIL() << "Invalid child PID returned by waitpid()";
        return;
      }
    }

    FAIL() << "Unable to open connection to dummy X server";
  }

#if 0
  d_->child_pid_ = vfork();
  if (d_->child_pid_ == -1) {
    FAIL() << "Failed to fork a process for dummy X server: "
    << std::strerror(errno);
  } else if (!d_->child_pid_) { /* Child */
    close(0);
    close(1);
    close(2);

    execlp("Xorg", "Xorg", display_string, "-config", d_->path_to_conf_.c_str(),
        NULL);
    perror("Failed to start dummy X server");
    exit(-1);
  } else { /* Parent */
    setenv("DISPLAY", display_string, true);

    for (int i = 0; i < 10; /*++i*/) {
      Display* display = XOpenDisplay(NULL);

      if (display) {
        XCloseDisplay(display);
        return;
      }

      int status;
      int pid = waitpid(d_->child_pid_, &status, WNOHANG);
      if (pid == d_->child_pid_) {
        d_->child_pid_ = -1;
        FAIL() << "Dummy X server failed to start, did you run as root?";
        return;
      } else if (pid == 0) {
        sleep(1); /* Give the dummy X server some time to start */
        continue;
      } else if (pid == -1) {
        FAIL() << "Could not get status of dummy X server process: "
        << std::strerror(errno);
        return;
      } else {
        FAIL() << "Invalid child PID returned by waitpid()";
        return;
      }
    }

    FAIL() << "Unable to open connection to dummy X server";
  }
#endif
}

void xorg::testing::Environment::TearDown() {
  if (!d_->process_.Terminate()) {
    FAIL() << "Warning: Failed to terminate dummy Xorg server: "
        << std::strerror(errno);
    if (!d_->process_.Kill())
      FAIL() << "Warning: Failed to kill dummy Xorg server: "
          << std::strerror(errno);
  }
#if 0
  if (d_->child_pid_ && d_->child_pid_ != -1) {
    if (kill(d_->child_pid_, SIGTERM) < 0) {
      FAIL() << "Warning: Failed to terminate dummy Xorg server: "
      << std::strerror(errno);
      if (kill(d_->child_pid_, SIGKILL))
      FAIL() << "Warning: Failed to kill dummy Xorg server: "
      << std::strerror(errno);
    }
  }
#endif
}
