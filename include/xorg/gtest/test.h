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

#ifndef XORG_GTEST_TEST_H_
#define XORG_GTEST_TEST_H_

#include <memory>

#include <gtest/gtest.h>
#include <X11/Xlib.h>

#include "utouch/frame.h"

namespace xorg {
namespace testing {

class Test : public ::testing::Test {
 public:
  Test();

  virtual void SetUp();
  virtual void TearDown();

 protected:
  ::Display* Display() const;

  struct Private;
  std::auto_ptr<Private> d_;

 private:
  /* Disable copy c'tor, assignment operator */
  Test(const Test&);
  Test& operator=(const Test&);
};

} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_TEST_H_
