/*******************************************************************************
 *
 * X testing environment - Google Test helper class to communicate with the
 * server
 *
 * Copyright (C) 2012 Red Hat, Inc.
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

#include "xorg/gtest/xorg-gtest-xserver.h"
#include "defines.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <map>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

struct xorg::testing::XServer::Private {
  Private()
      : display_number(DEFAULT_DISPLAY),
        path_to_server(DEFAULT_XORG_SERVER) {

    options["-config"] = DUMMY_CONF_PATH;
    options["-logfile"] = DEFAULT_XORG_LOGFILE;
  }

  unsigned int display_number;
  std::string display_string;
  std::string path_to_server;
  std::map<std::string, std::string> options;
};

xorg::testing::XServer::XServer() : d_(new Private) {
  d_->display_number = DEFAULT_DISPLAY;
  SetDisplayNumber(d_->display_number);
}

void xorg::testing::XServer::SetDisplayNumber(unsigned int display_number) {
    d_->display_number = display_number;

    std::stringstream s;
    s << ":" << display_number;
    d_->display_string = s.str();
}

unsigned int xorg::testing::XServer::GetDisplayNumber(void) {
  return d_->display_number;
}

const std::string& xorg::testing::XServer::GetDisplayString(void) {
  return d_->display_string;
}

void xorg::testing::XServer::SetServerPath(const std::string &path_to_server) {
  d_->path_to_server = path_to_server;
}

bool xorg::testing::XServer::WaitForEvent(::Display *display, time_t timeout)
{
    fd_set fds;
    FD_ZERO(&fds);

    int display_fd = ConnectionNumber(display);

    XSync(display, False);

    if (XPending(display))
        return true;
    else {
        FD_SET(display_fd, &fds);

        struct timeval timeval = {
            static_cast<time_t>(timeout / 1000),
            static_cast<time_t>(timeout % 1000),
        };

        int ret;
        if (timeout)
            ret = select(display_fd + 1, &fds, NULL, NULL, &timeval);
        else
            ret = select(display_fd + 1, &fds, NULL, NULL, NULL);

        if (ret < 0)
            throw std::runtime_error("Failed to select on X fd");

        if (ret == 0)
            return false;

        return XPending(display);
    }
}

bool xorg::testing::XServer::WaitForEventOfType(::Display *display, int type, int extension,
                                                int evtype, time_t timeout)
{
    while (WaitForEvent(display)) {
        XEvent event;
        if (!XPeekEvent(display, &event))
            throw std::runtime_error("Failed to peek X event");

        if (event.type != type) {
            if (XNextEvent(display, &event) != Success)
                throw std::runtime_error("Failed to remove X event");
            continue;
        }

        if (event.type != GenericEvent || extension == -1)
            return true;

        XGenericEvent *generic_event = reinterpret_cast<XGenericEvent*>(&event);

        if (generic_event->extension != extension) {
            if (XNextEvent(display, &event) != Success)
                throw std::runtime_error("Failed to remove X event");
            continue;
        }

        if (evtype == -1 || generic_event->evtype == evtype)
            return true;

        if (XNextEvent(display, &event) != Success)
            throw std::runtime_error("Failed to remove X event");
    }

    return false;
}

bool xorg::testing::XServer::WaitForDevice(::Display *display, const std::string &name,
                                           time_t timeout)
{
    int opcode;
    int event_start;
    int error_start;

    if (!XQueryExtension(display, "XInputExtension", &opcode, &event_start,
                         &error_start))
        throw std::runtime_error("Failed to query XInput extension");

    while (WaitForEventOfType(display, GenericEvent, opcode,
                              XI_HierarchyChanged)) {
        XEvent event;
        if (XNextEvent(display, &event) != Success)
            throw std::runtime_error("Failed to get X event");

        XGenericEventCookie *xcookie =
            reinterpret_cast<XGenericEventCookie*>(&event.xcookie);
        if (!XGetEventData(display, xcookie))
            throw std::runtime_error("Failed to get X event data");

        XIHierarchyEvent *hierarchy_event =
            reinterpret_cast<XIHierarchyEvent*>(xcookie->data);

        if (!(hierarchy_event->flags & XIDeviceEnabled)) {
            XFreeEventData(display, xcookie);
            continue;
        }

        bool device_found = false;
        for (int i = 0; i < hierarchy_event->num_info; i++) {
            if (!(hierarchy_event->info[i].flags & XIDeviceEnabled))
                continue;

            int num_devices;
            XIDeviceInfo *device_info =
                XIQueryDevice(display, hierarchy_event->info[i].deviceid,
                              &num_devices);
            if (num_devices != 1 || !device_info)
                throw std::runtime_error("Failed to query device");

            if (name.compare(device_info[0].name) == 0) {
                device_found = true;
                break;
            }
        }

        XFreeEventData(display, xcookie);

        if (device_found)
            return true;
    }

    return false;
}

void xorg::testing::XServer::WaitForConnections(void) {
  for (int i = 0; i < 10; ++i) {
    Display *test_display = XOpenDisplay(GetDisplayString().c_str());

    if (test_display) {
      XCloseDisplay(test_display);
      return;
    }

    int status;
    int pid = waitpid(Pid(), &status, WNOHANG);
    if (pid == Pid()) {
      std::string message;
      message += "X server failed to start on display ";
      message +=  GetDisplayString();
      message += ". Ensure that the \"dummy\" video driver is installed.\n"
                 "If the X.org server is older than 1.12, "
                 "tests will need to be run as root.\nCheck ";
      message += d_->options["-logfile"];
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

void xorg::testing::XServer::TestStartup(void) {
  Display* test_display = XOpenDisplay(GetDisplayString().c_str());
  if (test_display) {
    XCloseDisplay(test_display);
    std::string message;
    message += "A server is already running on ";
    message += GetDisplayString();
    message += ".";
    throw std::runtime_error(message);
  }

  /* The Xorg server won't start unless the log file and the old log file are
   * writable. */
  std::ofstream log_test;
  log_test.open(d_->options["-logfile"].c_str(), std::ofstream::out);
  log_test.close();
  if (log_test.fail()) {
    std::string message;
    message += "X.org server log file ";
    message += d_->options["-config"];
    message += " is not writable.";
    throw std::runtime_error(message);
  }

  std::string old_log_file = d_->options["-config"];
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

}

void xorg::testing::XServer::Start(const std::string &program) {
  TestStartup();

  std::vector<std::string> args;
  std::map<std::string, std::string>::iterator it;

  args.push_back(program);
  args.push_back(std::string(GetDisplayString()));

  for (it = d_->options.begin(); it != d_->options.end(); it++) {
    args.push_back(it->first);
    if (!it->second.empty())
      args.push_back(it->second);
  }

  Process::Start(program.empty() ? d_->path_to_server : program, args);
}

bool xorg::testing::XServer::Terminate(unsigned int timeout) {
  if (!Process::Terminate(timeout)) {
    std::cerr << "Warning: Failed to terminate dummy Xorg server: "
              << std::strerror(errno) << "\n";
    return false;
  } else
    return true;
}

bool xorg::testing::XServer::Kill(unsigned int timeout) {
  if (!Process::Kill(timeout)) {
    std::cerr << "Warning: Failed to kill dummy Xorg server: "
              << std::strerror(errno) << "\n";
    return false;
  } else
    return true;
}

void xorg::testing::XServer::SetOption(const std::string &key, const std::string &value) {
  d_->options[key] = value;
}
