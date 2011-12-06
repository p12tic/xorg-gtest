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

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <X11/Xlib.h>

xorg::testing::Environment::Environment(const std::string& path, int display)
    : path_to_conf_(path),
      display_(display),
      child_pid_(-1) {

}

void xorg::testing::Environment::SetUp() {
  static char display_string[6];
  snprintf(display_string, 6, ":%d", display_);

  child_pid_ = vfork();
  if (child_pid_ == -1) {
    FAIL() << "Failed to fork a process for dummy X server: "
        << std::strerror(errno);
  } else if (!child_pid_) { /* Child */
    close(0);
    close(1);
    close(2);

    execlp("Xorg", "Xorg", display_string, "-config", path_to_conf_.c_str(),
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
      int pid = waitpid(child_pid_, &status, WNOHANG);
      if (pid == child_pid_) {
        child_pid_ = -1;
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
}

void xorg::testing::Environment::TearDown() {
  if (child_pid_ && child_pid_ != -1) {
    if (kill(child_pid_, SIGTERM) < 0) {
      FAIL() << "Warning: Failed to terminate dummy Xorg server: "
          << std::strerror(errno);
      if (kill(child_pid_, SIGKILL))
        FAIL() << "Warning: Failed to kill dummy Xorg server: "
            << std::strerror(errno);
    }
  }
}
