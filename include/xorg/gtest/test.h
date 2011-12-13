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

#ifndef XORG_GTEST_TEST_H_
#define XORG_GTEST_TEST_H_

#include <memory>

#include <gtest/gtest.h>
#include <X11/Xlib.h>

namespace xorg {
namespace testing {

/**
 * @class Test test.h xorg/gtest/test.h
 *
 * Google %Test fixture providing an Xlib connection to an X11 server.
 *
 * Sets up and tears down an XLib connection to an X11 server.
 * Rely on Google %Test's TEST_F macro to use this fixture for your
 * own tests or subclass it and override the SetUp and TearDown
 * methods.
 *
 * @remark The display port is read from the environment variable DISPLAY.
 */
class Test : public ::testing::Test {
 public:

  Test();

  /**
   * Tries to connect to an X server instance.
   *
   * Fails if no X server is running. Updates the display object.
   * Reimplemented from ::testing::Test, should only be called by subclasses.
   * See Google %Test documentation for details.
   *
   * @post Subsequent calls to Display() return a valid pointer or NULL if an error occured.
   *
   * @throws std::runtime_error if no X server is running.
   */
  virtual void SetUp();

  /**
   * Closes the display.
   *
   * Reimplemented from ::testing::Test, should only be called by subclasses.
   * See Google %Test documentation for details.
   *
   * @post Subsequent calls to Display() return NULL.
   */
  virtual void TearDown();

 protected:

  /**
   * Accesses the display representing an Xlib connection.
   *
   * Accessible by subclasses and test cases relying on this fixture.
   *
   * @returns Pointer to a display or NULL.
   */
  ::Display* Display() const;

  /** @cond Implementation */
  struct Private;
  std::auto_ptr<Private> d_;
  /** @endcond Implementation */
 private:
  /* Disable copy c'tor, assignment operator */
  Test(const Test&);
  Test& operator=(const Test&);
};

} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_TEST_H_
