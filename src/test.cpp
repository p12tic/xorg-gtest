/*****************************************************************************
 *
 * utouch-frame - Touch Frame Library
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

#include "xorg/gtest/test.h"

#include <X11/Xlib.h>

struct xorg::testing::Test::Private {
  ::Display* display;
};

xorg::testing::Test::Test() : d_(new Private) {
  d_->display = NULL;
}

void xorg::testing::Test::SetUp() {
  d_->display = XOpenDisplay(NULL);
  ASSERT_TRUE(d_->display != NULL) << "Failed to open connection to display";
}

void xorg::testing::Test::TearDown() {
  XCloseDisplay(d_->display);
  d_->display = NULL;
}

::Display* xorg::testing::Test::Display() const {
  return d_->display;
}

xorg::testing::Test::~Test() {
  delete d_;
}
