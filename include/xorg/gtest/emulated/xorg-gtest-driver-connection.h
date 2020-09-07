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

#ifndef XORG_GTEST_EMULATED_DRIVER_CONNECTION_H_
#define XORG_GTEST_EMULATED_DRIVER_CONNECTION_H_

#include <memory>
#include <string>
#include "xorg-gtest-valuators.h"
#include <xorg/emulated-events.h>

namespace xorg {
namespace testing {
namespace emulated {

/**
 * A connection to replay events through xf86-input-emulated input driver
 */

class DriverConnection {
 public:
  /**
   * Create a new driver connection.
   *
   * @param [in] events_path Path to the FIFO that the driver will read events from
   * @param [in] responses_path Path to the FIFO that the driver will write responses to
   */
  explicit DriverConnection(const std::string& events_path, const std::string& responses_path);
  ~DriverConnection();

  /**
   * Returns the events input path
   */
  const std::string& EventsInPath() const;

  /**
   * Returns responses output path
   */
  const std::string& ResponsesOutPath() const;

  /**
   * Waits for the FIFO files being created and opens them.
   * Throws an exception if the files are not available after 10 seconds.
   */
  void WaitOpen();

  /**
   * Closes the FIFO files if they are open
   */
  void Close();

  /**
   * All of the functions below result in one input event being sent to the X server
   * via the FIFO files. If the files are not open yet, an exception is thrown.
   */
  void PlayRelMotion(const Valuators& valuators);

  void PlayAbsMotion(const Valuators& valuators);

  void PlayProximityIn(const Valuators& valuators);
  void PlayProximityOut(const Valuators& valuators);

  void PlayButtonDownAbs(std::int32_t button, const Valuators& valuators);
  void PlayButtonDownRel(std::int32_t button, const Valuators& valuators);

  void PlayButtonUpAbs(std::int32_t button, const Valuators& valuators);
  void PlayButtonUpRel(std::int32_t button, const Valuators& valuators);

  void PlayKeyDown(std::int32_t key_code);
  void PlayKeyUp(std::int32_t key_code);

  void PlayTouchBegin(std::uint32_t id, const Valuators& valuators);
  void PlayTouchUpdate(std::uint32_t id, const Valuators& valuators);
  void PlayTouchEnd(std::uint32_t id, const Valuators& valuators);

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

  /**
   * Synchronizes with the X server. Sends a synchronization event and waits for response.
   * This is done after all input events and is thus unnecessary in regular circumstances.
   */
  void Sync();

  struct Private;
  std::unique_ptr<Private> d_;

  void EnsureOpen();
};

} // namespace emulated
} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_EMULATED_DRIVER_CONNECTION_H_
