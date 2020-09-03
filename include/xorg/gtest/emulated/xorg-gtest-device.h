/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2020 Povilas Kanapickas <povilas@radix.lt>
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

#ifndef XORG_GTEST_EMULATED_DEVICE_H_
#define XORG_GTEST_EMULATED_DEVICE_H_

#include <memory>
#include <string>
#include "xorg-gtest-valuators.h"
#include <xorg/emulated-events.h>

namespace xorg {
namespace testing {
namespace emulated {

enum class DeviceType {
  KEYBOARD,
  POINTER,
  POINTER_ABSOLUTE,
  POINTER_GESTURE,
  POINTER_ABSOLUTE_PROXIMITY,
  TOUCH
};

/**
 * A device to replay events through xf86-input-emulated input driver
 */
class Device {
 public:
  /**
   * Create a new device context.
   *
   * @param [in] path_prefix Path prefix according to which the paths for the FIFOs will be
   * contructed. The FIFOs are then used for communication with the emulated driver
   * @param [in] type The type of the device which defines what operations are available.
   */
  explicit Device(const std::string& path_prefix, DeviceType type);
  ~Device();

  /**
   * Returns the options that the server needs to be configured for this device.
   */
  std::string GetOptions() const;

  /**
   * Waits for the connection to the driver being established.
   * Throws an exception if the files are not available after 10 seconds.
   */
  void WaitOpen();

  /**
   * All of the functions below result in an input event being sent to the X server.
   * WaitOpen() must be called before using them.
   */
  void PlayRelMotion(double x, double y);
  void PlayRelMotion(double x, double y, const Valuators& extra_valuators);

  void PlayAbsMotion(double x, double y);
  void PlayAbsMotion(double x, double y, const Valuators& extra_valuators);

  void PlayProximityIn(const Valuators& valuators);
  void PlayProximityOut(const Valuators& valuators);

  void PlayButtonDown(std::int32_t button);
  void PlayButtonUp(std::int32_t button);

  void PlayKeyDown(std::int32_t key_code);
  void PlayKeyUp(std::int32_t key_code);

  void PlayTouchBegin(double x, double y, std::uint32_t id);
  void PlayTouchBegin(double x, double y, std::uint32_t id, const Valuators& extra_valuators);
  void PlayTouchUpdate(double x, double y, std::uint32_t id);
  void PlayTouchUpdate(double x, double y, std::uint32_t id, const Valuators& extra_valuators);
  void PlayTouchEnd(double x, double y, std::uint32_t id);
  void PlayTouchEnd(double x, double y, std::uint32_t id, const Valuators& extra_valuators);

  void PlayGestureSwipeBegin(std::uint16_t num_touches, double delta_x, double delta_y,
                             double delta_unaccel_x, double delta_unaccel_y);
  void PlayGestureSwipeUpdate(std::uint16_t num_touches, double delta_x, double delta_y,
                              double delta_unaccel_x, double delta_unaccel_y);
  void PlayGestureSwipeEnd(std::uint16_t num_touches, double delta_x, double delta_y,
                           double delta_unaccel_x, double delta_unaccel_y);
  void PlayGestureSwipeCancel(std::uint16_t num_touches, double delta_x, double delta_y,
                              double delta_unaccel_x, double delta_unaccel_y);

  void PlayGesturePinchBegin(std::uint16_t num_touches, double delta_x, double delta_y,
                             double delta_unaccel_x, double delta_unaccel_y,
                             double scale, double delta_angle);
  void PlayGesturePinchUpdate(std::uint16_t num_touches, double delta_x, double delta_y,
                              double delta_unaccel_x, double delta_unaccel_y,
                              double scale, double delta_angle);
  void PlayGesturePinchEnd(std::uint16_t num_touches, double delta_x, double delta_y,
                           double delta_unaccel_x, double delta_unaccel_y,
                           double scale, double delta_angle);
  void PlayGesturePinchCancel(std::uint16_t num_touches, double delta_x, double delta_y,
                              double delta_unaccel_x, double delta_unaccel_y,
                              double scale, double delta_angle);
 private:

  struct Private;
  std::unique_ptr<Private> d_;
};

} // namespace emulated
} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_EMULATED_DEVICE_H_
