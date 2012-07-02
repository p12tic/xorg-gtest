/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2012 Canonical Ltd.
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

#include <fcntl.h>
#include <dirent.h>

#include <stdexcept>

#include <gtest/gtest.h>

#define SYS_INPUT_DIR "/sys/class/input"
#define DEV_INPUT_DIR "/dev/input/"

struct xorg::testing::evemu::Device::Private {
  Private() : fd(-1), device(NULL), device_node() {}

  int fd;
  struct evemu_device* device;
  std::string device_node;
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

void xorg::testing::evemu::Device::GuessDeviceNode(time_t ctime) {
  struct dirent **event_devices;
  int n_event_devices;

  n_event_devices = scandir(SYS_INPUT_DIR, &event_devices,
                            _event_device_filter, _event_device_compare);

  if (n_event_devices < 0) {
    std::cerr << "Failed to guess device node." << std::endl;
    return;
  }

  bool found = false;
  for (int i = 0; i < n_event_devices && !found; i++) {
    std::stringstream s;
    s << DEV_INPUT_DIR << event_devices[i]->d_name;

    int fd = open(s.str().c_str(), O_RDONLY);
    char device_name[256];

    ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name);
    if (strcmp(device_name, evemu_get_name(d_->device)) == 0) {
      struct stat buf;
      if (fstat(fd, &buf) == 0) {
        if (buf.st_ctime >= ctime) {
          d_->device_node = s.str();
          found = true;
        }
      }
    }
    close(fd);
  }

  for (int i = 0; i < n_event_devices; i++)
    free(event_devices[i]);
  free(event_devices);
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

  d_->fd = open(UINPUT_NODE, O_WRONLY);
  if (d_->fd < 0) {
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to open uinput node");
  }

  time_t ctime = time(NULL);
  if (evemu_create(d_->device, d_->fd) < 0) {
    close(d_->fd);
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to create evemu device");
  }

  GuessDeviceNode(ctime);
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

const std::string& xorg::testing::evemu::Device::GetDeviceNode(void) {
  return d_->device_node;
}

xorg::testing::evemu::Device::~Device() {
  close(d_->fd);
  evemu_delete(d_->device);
}
