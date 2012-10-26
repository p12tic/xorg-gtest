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
#include <X11/Xlibint.h>
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
  std::string version;
};

xorg::testing::XServer::XServer() : d_(new Private) {
  d_->display_number = DEFAULT_DISPLAY;
  SetDisplayNumber(d_->display_number);
}

xorg::testing::XServer::~XServer() {
  if (Pid() > 0)
    if (!Terminate(3000))
      Kill(300);
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
    while (WaitForEvent(display, timeout)) {
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

static XIEventMask* set_hierarchy_mask(::Display *display,
                                       int *nmasks_out,
                                       bool *was_set,
                                       bool *was_created)
{
    XIEventMask *masks;
    int nmasks;
    bool mask_toggled = false;
    bool new_mask_created = false;
    XIEventMask *all_devices_mask = NULL;

    masks = XIGetSelectedEvents(display, DefaultRootWindow(display), &nmasks);

    /* masks is in a quirky data format (one chunk of memory). Change into a
       format easier to manipulate. */

    /* extra one, in case we have zero masks or no XIAllDevices mask */
    XIEventMask *new_masks = new XIEventMask[nmasks + 1];
    for (int i = 0; i < nmasks; i++) {
      XIEventMask *m = &new_masks[i];
      *m = masks[i];

      if (masks[i].deviceid == XIAllDevices) {
        all_devices_mask = m;
        if (masks[i].mask_len < XIMaskLen(XI_HierarchyChanged)) {
          m->mask_len = XIMaskLen(XI_HierarchyChanged);
          mask_toggled = true;
        } else
          mask_toggled = !XIMaskIsSet(m->mask, XI_HierarchyChanged);
      }

      m->mask = new unsigned char[m->mask_len]();
      memcpy(m->mask, masks[i].mask, masks[i].mask_len);

      if (mask_toggled && m->deviceid == XIAllDevices)
        XISetMask(m->mask, XI_HierarchyChanged);
    }

    if (!all_devices_mask) {
      all_devices_mask = &new_masks[nmasks++];
      all_devices_mask->deviceid = XIAllDevices;
      all_devices_mask->mask_len = XIMaskLen(XI_HierarchyChanged);
      all_devices_mask->mask = new unsigned char[all_devices_mask->mask_len]();
      XISetMask(all_devices_mask->mask, XI_HierarchyChanged);
      new_mask_created = true;
    }

    XFree(masks);
    masks = NULL;

    if (new_mask_created || mask_toggled) {
      XISelectEvents(display, DefaultRootWindow(display), new_masks, nmasks);
      XFlush(display);
    }

    *was_set = mask_toggled;
    *was_created = new_mask_created;
    *nmasks_out = nmasks;

    return new_masks;
}

static void unset_hierarchy_mask(::Display *display,
                                 XIEventMask *masks, int nmasks,
                                 bool was_set, bool was_created)
{
    if (was_set || was_created) {
      if (was_set) {
        for (int i = 0; i < nmasks; i++) {
          if (masks[i].deviceid == XIAllDevices)
            XIClearMask(masks[i].mask, XI_HierarchyChanged);
        }
      } else if (was_created)
        masks[nmasks - 1].mask_len = 0;
      XISelectEvents(display, DefaultRootWindow(display), masks, nmasks);
      XFlush(display);
    }

    for (int i = 0; i < nmasks; i++)
      delete[] masks[i].mask;
    delete[] masks;
}

bool xorg::testing::XServer::WaitForDevice(::Display *display, const std::string &name,
                                           time_t timeout)
{
    int opcode;
    int event_start;
    int error_start;
    bool device_found = false;

    if (!XQueryExtension(display, "XInputExtension", &opcode, &event_start,
                         &error_start))
        throw std::runtime_error("Failed to query XInput extension");

    XIEventMask *masks;
    int nmasks;
    bool mask_set, mask_created;
    masks = set_hierarchy_mask(display, &nmasks, &mask_set, &mask_created);

    XIDeviceInfo *info;
    int ndevices;

    info = XIQueryDevice(display, XIAllDevices, &ndevices);
    for (int i = 0; !device_found && i < ndevices; i++) {
      device_found = (name.compare(info[i].name) == 0);
    }
    XIFreeDeviceInfo(info);

    while (!device_found &&
           WaitForEventOfType(display, GenericEvent, opcode,
                              XI_HierarchyChanged, timeout)) {
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

        device_found = false;
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
          break;
    }

    unset_hierarchy_mask(display, masks, nmasks, mask_set, mask_created);

    return device_found;
}

void xorg::testing::XServer::WaitForConnections(void) {
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

  std::string log = d_->options["-logfile"];

  /* The Xorg server won't start unless the log file and the old log file are
   * writable. */
  bool logfile_was_present;
  std::ifstream file_test;
  file_test.open(log.c_str());
  logfile_was_present = file_test.good();

  std::ofstream log_test;
  log_test.open(log.c_str(), std::ofstream::out);
  log_test.close();
  if (log_test.fail()) {
    throw std::runtime_error("X.org server log file " + log + " is not writable.");
  } else if (!logfile_was_present)
    unlink(log.c_str());

  std::string old_log_file = log + ".old";

  file_test.open(old_log_file.c_str());
  logfile_was_present = file_test.good();

  log_test.open(old_log_file.c_str(), std::ofstream::out);
  log_test.close();
  if (log_test.fail()) {
    throw std::runtime_error("X.org old server log file " + old_log_file + " is not writable.");
  } else if (!logfile_was_present)
    unlink(old_log_file.c_str());
}

const std::string& xorg::testing::XServer::GetVersion(void) {
  if (Pid() == -1 || !d_->version.empty())
    return d_->version;

  std::ifstream logfile;
  logfile.open(d_->options["-logfile"].c_str());

  std::string prefix = "X.Org X Server ";

  if (logfile.is_open()) {
    std::string line;
    while (getline(logfile, line)) {
      size_t start = line.find(prefix);
      if (start == line.npos)
        continue;

      line = line.substr(prefix.size());
      /* RCs have the human-readable version after the version */
      size_t end = line.find(" ");
      if (end == line.npos)
        end = line.size();

      d_->version = line.substr(0, end);
      break;
    }
  }

  return d_->version;
}

static int _x_io_error_handler(Display *dpy) _X_NORETURN;
static int _x_io_error_handler(Display *dpy)
{
  throw xorg::testing::XIOError("Connection to X Server lost. Possible server crash.");
}

void xorg::testing::XServer::RegisterXIOErrorHandler()
{
  XIOErrorHandler old_handler;
  old_handler = XSetIOErrorHandler(_x_io_error_handler);

  if (old_handler != _XDefaultIOError)
    XSetIOErrorHandler(old_handler);
}

void xorg::testing::XServer::Start(const std::string &program) {
  TestStartup();

  std::vector<std::string> args;
  std::map<std::string, std::string>::iterator it;
  std::string err_msg;

  sigset_t sig_mask;
  struct timespec sig_timeout = {3, 0}; /* 3 sec + 0 nsec */

  /* add SIGUSR1 to the signal mask */
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGUSR1);
  sigaddset(&sig_mask, SIGCHLD);
  if (sigprocmask(SIG_BLOCK, &sig_mask, NULL)) {
    err_msg.append("Failed to set signal mask: ");
    err_msg.append(std::strerror(errno));
    throw std::runtime_error(err_msg);
  }

  pid_t pid = Fork();
  if (pid == 0) {
#ifdef __linux
    if (getenv("XORG_GTEST_XSERVER_KEEPALIVE"))
      prctl(PR_SET_PDEATHSIG, 0);
#endif

    /* set SIGUSR1 handler to SIG_IGN, XServer tests for this and will
     * send SIGUSR1 when ready */
    sighandler_t old_handler;
    old_handler = signal(SIGUSR1, SIG_IGN);
    if (old_handler == SIG_ERR) {
      err_msg.append("Failed to set signal handler: ");
      err_msg.append(std::strerror(errno));
      throw std::runtime_error(err_msg);
    }

    /* unblock for the child process so the server receives SIGUSR1, needed
       for VT switching */
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    if (sigprocmask(SIG_UNBLOCK, &sig_mask, NULL)) {
      err_msg.append("Failed to unblock signal mask: ");
      err_msg.append(std::strerror(errno));
      throw std::runtime_error(err_msg);
    }

    args.push_back(std::string(GetDisplayString()));

    for (it = d_->options.begin(); it != d_->options.end(); it++) {
      args.push_back(it->first);
      if (!it->second.empty())
        args.push_back(it->second);
    }

    Process::Start(program.empty() ? d_->path_to_server : program, args);
    /* noreturn */

  }

  /* parent */
  char *sleepwait = getenv("XORG_GTEST_XSERVER_SIGSTOP");
  if (sleepwait)
    raise(SIGSTOP);

  /* wait for SIGUSR1 from XServer */
  int recv_sig = sigtimedwait(&sig_mask, NULL, &sig_timeout);
  if (recv_sig == SIGCHLD) {
    GetState();
  } else if (recv_sig != SIGUSR1 && errno != EAGAIN) {
    err_msg.append("Error while waiting for XServer startup: ");
    err_msg.append(std::strerror(errno));
    throw std::runtime_error(err_msg);
  }

  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGCHLD);
  sigaddset(&sig_mask, SIGUSR1);
  sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);

  RegisterXIOErrorHandler();
}

bool xorg::testing::XServer::Terminate(unsigned int timeout) {
  if (getenv("XORG_GTEST_XSERVER_KEEPALIVE"))
    return true;

  if (!Process::Terminate(timeout)) {
    std::cerr << "Warning: Failed to terminate Xorg server: "
              << std::strerror(errno) << "\n";
    return false;
  } else
    return true;
}

bool xorg::testing::XServer::Kill(unsigned int timeout) {
  if (getenv("XORG_GTEST_XSERVER_KEEPALIVE"))
    return true;

  if (!Process::Kill(timeout)) {
    std::cerr << "Warning: Failed to kill Xorg server: "
              << std::strerror(errno) << "\n";
    return false;
  } else
    return true;
}

void xorg::testing::XServer::RemoveLogFile(bool force) {
  enum Process::State state = GetState();
  if (force || state == Process::TERMINATED || state == Process::FINISHED_SUCCESS)
    unlink(d_->options["-logfile"].c_str());
}

void xorg::testing::XServer::SetOption(const std::string &key, const std::string &value) {
  d_->options[key] = value;
}

const std::string& xorg::testing::XServer::GetLogFilePath() {
  return d_->options["-logfile"];
}

const std::string& xorg::testing::XServer::GetConfigPath() {
  return d_->options["-config"];
}
