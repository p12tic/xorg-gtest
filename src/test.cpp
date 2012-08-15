/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011, 2012 Canonical Ltd.
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

#include "xorg/gtest/xorg-gtest-test.h"

#include <stdexcept>

#include <X11/Xlib.h>

struct xorg::testing::Test::Private {
  ::Display* display;
  std::string display_string;
};

xorg::testing::Test::Test() : d_(new Private) {
  d_->display = NULL;
}

xorg::testing::Test::~Test() {}

void xorg::testing::Test::SetUp() {
  const char *dpy = NULL;

  if (!d_->display_string.empty())
    dpy = d_->display_string.c_str();

  d_->display = XOpenDisplay(dpy);
  if (!d_->display)
    throw std::runtime_error("Failed to open connection to display");
}

void xorg::testing::Test::TearDown() {
  if (d_->display)
    XCloseDisplay(d_->display);
  d_->display = NULL;
}

::Display* xorg::testing::Test::Display() const {
  return d_->display;
}

void xorg::testing::Test::SetDisplayString(const std::string &display) {
  d_->display_string = display;
}
