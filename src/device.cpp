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

#include <stdexcept>

#include <gtest/gtest.h>

struct xorg::testing::evemu::Device::Private {
  Private() : fd(-1), device(NULL) {}

  int fd;
  struct evemu_device* device;
};

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

  if (evemu_create(d_->device, d_->fd) < 0) {
    close(d_->fd);
    evemu_delete(d_->device);
    throw std::runtime_error("Failed to create evemu device");
  }
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

xorg::testing::evemu::Device::~Device() {
  close(d_->fd);
  evemu_delete(d_->device);
}
