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

#ifndef XORG_GTEST_EVEMU_DEVICE_H_
#define XORG_GTEST_EVEMU_DEVICE_H_

#include <memory>
#include <string>

extern "C" {

#include <evemu.h>

} // extern "C"

namespace xorg {
namespace testing {
namespace evemu {

/**
 * @class Device device.h xorg/gtest/evemu/device.h
 *
 * uTouch-Evemu input device for replaying events through the Linux uinput
 * evdev subsystem.
 *
 * Use the Recording class to play back a specific recording.
 */

class Device {
 public:
  /**
   * Create a new device context.
   *
   * @param [in] path Path to uTouch-Evemu device property file.
   *
   * @throws std::runtime_error if the device property file could not be found
   *         or the device could not be created.
   */
  explicit Device(const std::string& path);
  ~Device();

  /**
   * Play a uTouch-Evemu recording through the device.
   *
   * Plays the recording from the beginning through the end. This call will
   * block until the recording has finished.
   *
   * @param [in] path Path to uTouch-Evemu recording file.
   *
   * @throws std::runtime_error if playback failed for any reason.
   */
  void Play(const std::string& path) const;

 private:
  struct Private;
  std::auto_ptr<Private> d_;

  /* Disable copy constructor & assignment operator */
  Device(const Device&);
  Device& operator=(const Device&);
};

} // namespace evemu
} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_EVEMU_DEVICE_H_
