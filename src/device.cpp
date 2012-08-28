/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2012 Canonical Ltd.
 * Copyright © 2012 Red Hat, Inc.
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

#include "xorg/gtest/evemu/xorg-gtest-device.h"

#include <linux/input.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <poll.h>

#include <stdexcept>

#include <gtest/gtest.h>

#define SYS_INPUT_DIR "/sys/class/input"
#define DEV_INPUT_DIR "/dev/input/"

struct xorg::testing::evemu::Device::Private {
  Private() : fd(-1), device(NULL), device_node() {}

  int fd;
  struct evemu_device* device;
  std::string device_node;
  time_t ctime;
};

static int _event_device_compare(const struct dirent **a,
                                 const struct dirent **b) {
  int na, nb;

  sscanf((*a)->d_name, "event%d", &na);
  sscanf((*b)->d_name, "event%d", &nb);

  return (na > nb) ? 1 : (na < nb) ? -1 : 0;

}

static int _event_device_filter(const struct dirent *d) {
  return (strncmp("event", d->d_name, sizeof("event") - 1) == 0);
}

static bool event_is_device(const std::string &path,
                            const std::string &devname,
                            time_t ctime) {
    char device_name[256];
    bool equal = false;
    int fd = open(path.c_str(), O_RDONLY);

    if (fd == -1)
      return false;

    if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) != -1 &&
        devname.compare(device_name) == 0) {
      struct stat buf;

      if (fstat(fd, &buf) == 0)
        if (buf.st_ctime >= ctime)
          equal = true;
    }
    close(fd);

    return equal;
}

void xorg::testing::evemu::Device::GuessDeviceNode(time_t ctime) {
  struct dirent **event_devices = NULL;
  int n_event_devices;

  n_event_devices = scandir(SYS_INPUT_DIR, &event_devices,
                            _event_device_filter, _event_device_compare);

  bool found = false;
  for (int i = 0; i < n_event_devices && !found; i++) {
    std::stringstream s;
    s << DEV_INPUT_DIR << event_devices[i]->d_name;
    found = event_is_device(s.str(), evemu_get_name(d_->device), ctime);
    if (found)
      d_->device_node = s.str();
  }

  if (!found)
    std::cerr << "Failed to guess device node." << std::endl;

  for (int i = 0; i < n_event_devices; i++)
    free(event_devices[i]);
  free(event_devices);
}

static std::string wait_for_inotify(int fd)
{
  std::string devnode;
  bool found = false;
  struct pollfd pfd;

  pfd.fd = fd;
  pfd.events = POLLIN;

  char buf[1024];
  size_t bufidx = 0;

  while (!found && poll(&pfd, 1, 2000) > 0) {
    ssize_t r;

    r = read(fd, buf + bufidx, sizeof(buf) - bufidx);
    if (r == -1 && errno != EAGAIN) {
      std::cerr << "inotify read failed with: " << std::string(strerror(errno)) << std::endl;
      break;
    }

    bufidx += r;

    struct inotify_event *e = reinterpret_cast<struct inotify_event*>(buf);

    while (bufidx > sizeof(*e) && bufidx >= sizeof(*e) + e->len) {
      if (strncmp(e->name, "event", 5) == 0) {
        devnode = DEV_INPUT_DIR + std::string(e->name);
        found = true;
        break;
      }

      /* this packet didn't contain what we're looking for */
      int len = sizeof(*e) + e->len;
      memmove(buf, buf + len, bufidx - len);
      bufidx -= len;
    }
  }

  return devnode;
}

xorg::testing::evemu::Device::Device(const std::string& path)
    : d_(new Private) {
  static const char UINPUT_NODE[] = "/dev/uinput";

  d_->device = evemu_new(NULL);
  if (!d_->device)
    throw std::runtime_error("Failed to create evemu record");

  FILE* fp = fopen(path.c_str(), "r");
  if (fp == NULL) {
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to open device file");
  }

  if (evemu_read(d_->device, fp) <= 0) {
    fclose(fp);
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to read device file");
  }

  fclose(fp);

  int ifd = inotify_init1(IN_NONBLOCK);
  if (ifd == -1 || inotify_add_watch(ifd, DEV_INPUT_DIR, IN_CREATE) == -1) {
    std::cerr << "Failed to create inotify watch" << std::endl;
    if (ifd != -1)
      close(ifd);
    ifd = -1;
  }

  d_->fd = open(UINPUT_NODE, O_WRONLY);
  if (d_->fd < 0) {
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to open uinput node");
  }

  d_->ctime = time(NULL);
  if (evemu_create(d_->device, d_->fd) < 0) {
    close(d_->fd);
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to create evemu device");
  }

  if (ifd != -1) {
    std::string devnode = wait_for_inotify(ifd);
    if (event_is_device(devnode, evemu_get_name(d_->device), d_->ctime))
        d_->device_node = devnode;
    close(ifd);
  } /* else guess node when we'll need it */
}

void xorg::testing::evemu::Device::Play(const std::string& path) const {
  FILE* file = fopen(path.c_str(), "r");
  if (!file)
    throw std::runtime_error("Failed to open recording file");

  if (evemu_play(file, d_->fd) != 0) {
    fclose(file);
    throw std::runtime_error("Failed to play evemu recording");
  }

  fclose(file);
}

void xorg::testing::evemu::Device::PlayOne(int type, int code, int value, bool sync)
{
  struct input_event ev;
  if (evemu_create_event(&ev, type, code, value))
    throw std::runtime_error("Failed to create event");

  if (evemu_play_one(d_->fd, &ev))
    throw std::runtime_error("Failed to play event");

  if (sync) {
    if (evemu_create_event(&ev, EV_SYN, SYN_REPORT, 0))
      throw std::runtime_error("Failed to create EV_SYN event");

    if (evemu_play_one(d_->fd, &ev))
      throw std::runtime_error("Failed to play EV_SYN event");
  }
}

const std::string& xorg::testing::evemu::Device::GetDeviceNode(void) {
  if (d_->device_node.empty())
    GuessDeviceNode(d_->ctime);
  return d_->device_node;
}

xorg::testing::evemu::Device::~Device() {
  close(d_->fd);
  evemu_delete(d_->device);
}
