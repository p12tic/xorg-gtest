/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011, 2012 Canonical Ltd.
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

#ifndef XORG_GTEST_TEST_H_
#define XORG_GTEST_TEST_H_

#include <memory>

#include <gtest/gtest.h>
#include <X11/Xlib.h>

namespace xorg {
namespace testing {

/**
 * @class Test xorg-gtest-test.h xorg/gtest/xorg-gtest-test.h
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

  virtual ~Test();

 protected:
  /**
   * Tries to connect to an X server instance.
   *
   * Fails if no X server is running. Updates the display object.
   * Reimplemented from ::testing::Test. See Google %Test documentation for
   * details.
   *
   * @post Subsequent calls to Display() return a valid pointer or NULL if an error occured.
   *
   * @throws std::runtime_error if no X server is running.
   */
  virtual void SetUp();

  /**
   * Closes the display.
   *
   * Reimplemented from ::testing::Test. See Google %Test documentation for
   * details.
   *
   * @post Subsequent calls to Display() return NULL.
   */
  virtual void TearDown();

  /**
   * Accesses the display representing an Xlib connection.
   *
   * Accessible by subclasses and test cases relying on this fixture.
   *
   * @returns Pointer to a display or NULL.
   */
  ::Display* Display() const;

  /**
   * Set the display string used for XOpenDisplay() and thus affects
   * Test::Display(). This function must be called before
   * xorg::testing::Test::SetUp() to have any effect.
   *
   * @param display The string representing the display connection, or an
   * empty string for NULL
   */
  void SetDisplayString(const std::string &display);

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
