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

#include "xorg/gtest/emulated/xorg-gtest-device.h"
#include "xorg/gtest/emulated/xorg-gtest-driver-connection.h"

#include <X11/extensions/XInput2.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <thread>

namespace xorg {
namespace testing {
namespace emulated {

namespace {

std::string DeviceTypeToOptionString(DeviceType type)
{
  switch (type) {
    case DeviceType::KEYBOARD: return "Keyboard";
    case DeviceType::POINTER: return "Pointer";
    case DeviceType::POINTER_GESTURE: return "PointerGesture";
    case DeviceType::POINTER_ABSOLUTE: return "PointerAbsolute";
    case DeviceType::POINTER_ABSOLUTE_PROXIMITY: return "PointerAbsoluteProximity";
    case DeviceType::TOUCH: return "Touch";
  }
  return "";
}

void EnsureDeviceTypeForEvent(DeviceType type, std::initializer_list<DeviceType> allowed_types)
{
  for (auto allowed_type : allowed_types) {
    if (type == allowed_type)
      return;
  }
  throw std::runtime_error("Unsupported event for the device type");
}

} // namespace

struct Device::Private {
  Private(const std::string& events_path, const std::string& responses_path) :
    connection(events_path, responses_path) {}

  DriverConnection connection;
  DeviceType device_type;
};

Device::Device(const std::string& path_prefix, DeviceType type) :
  d_{std::unique_ptr<Private>(new Private(path_prefix + "events_in",
                                          path_prefix + "responses_out"))}
{
  d_->device_type = type;
}

Device::~Device()
{
}

std::string Device::GetOptions() const
{
  // add a new line before to protect against accidentally missed newline in the rest of
  // configuration
  return "\n"
      "Option \"EmulatedType\" \"" + DeviceTypeToOptionString(d_->device_type) + "\"\n"
      "Option \"EventsInPath\" \"" + d_->connection.EventsInPath() + "\"\n"
      "Option \"EventsOutPath\" \"" + d_->connection.ResponsesOutPath() + "\"\n";
}

void Device::WaitOpen()
{
  d_->connection.WaitOpen();
}

void Device::PlayRelMotion(double x, double y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER, DeviceType::POINTER_GESTURE});
  Valuators valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayRelMotion(valuators);
}

void Device::PlayRelMotion(double x, double y, const Valuators& extra_valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER, DeviceType::POINTER_GESTURE});
  Valuators valuators = extra_valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayRelMotion(valuators);
}

void Device::PlayAbsMotion(double x, double y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_ABSOLUTE});
  Valuators valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayAbsMotion(valuators);
}

void Device::PlayAbsMotion(double x, double y, const Valuators& extra_valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_ABSOLUTE});
  Valuators valuators = extra_valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayAbsMotion(valuators);
}

void Device::PlayProximityIn(const Valuators& valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_ABSOLUTE_PROXIMITY});
  d_->connection.PlayProximityIn(valuators);
}

void Device::PlayProximityOut(const Valuators& valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_ABSOLUTE_PROXIMITY});
  d_->connection.PlayProximityOut(valuators);
}

void Device::PlayButtonDown(std::int32_t button)
{
  EnsureDeviceTypeForEvent(d_->device_type, {
    DeviceType::POINTER,
    DeviceType::POINTER_GESTURE,
    DeviceType::POINTER_ABSOLUTE,
    DeviceType::POINTER_ABSOLUTE_PROXIMITY,
  });
  if (button == 0 || button >= 256)
    throw std::runtime_error("Invalid button number");

  if (d_->device_type == DeviceType::POINTER || d_->device_type == DeviceType::POINTER_GESTURE) {
    d_->connection.PlayButtonDownRel(button, Valuators());
  } else {
    d_->connection.PlayButtonDownAbs(button, Valuators());
  }
}

void Device::PlayButtonUp(std::int32_t button)
{
  EnsureDeviceTypeForEvent(d_->device_type, {
    DeviceType::POINTER,
    DeviceType::POINTER_GESTURE,
    DeviceType::POINTER_ABSOLUTE,
    DeviceType::POINTER_ABSOLUTE_PROXIMITY,
  });
  if (button == 0 || button >= 256)
    throw std::runtime_error("Invalid button number");

  if (d_->device_type == DeviceType::POINTER || d_->device_type == DeviceType::POINTER_GESTURE) {
    d_->connection.PlayButtonUpRel(button, Valuators());
  } else {
    d_->connection.PlayButtonUpAbs(button, Valuators());
  }
}

void Device::PlayKeyDown(std::int32_t key_code)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::KEYBOARD});
  d_->connection.PlayKeyDown(key_code);
}

void Device::PlayKeyUp(std::int32_t key_code)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::KEYBOARD});
  d_->connection.PlayKeyUp(key_code);
}

void Device::PlayTouchBegin(double x, double y, std::uint32_t id)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchBegin(id, valuators);
}

void Device::PlayTouchBegin(double x, double y, std::uint32_t id, const Valuators& extra_valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators = extra_valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchBegin(id, valuators);
}

void Device::PlayTouchUpdate(double x, double y, std::uint32_t id)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchUpdate(id, valuators);
}

void Device::PlayTouchUpdate(double x, double y, std::uint32_t id, const Valuators& extra_valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators = extra_valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchUpdate(id, valuators);
}

void Device::PlayTouchEnd(double x, double y, std::uint32_t id)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchEnd(id, valuators);
}

void Device::PlayTouchEnd(double x, double y, std::uint32_t id, const Valuators& extra_valuators)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::TOUCH});
  Valuators valuators = extra_valuators;
  valuators.Set(0, x);
  valuators.Set(1, y);
  d_->connection.PlayTouchEnd(id, valuators);
}

void Device::PlayGestureSwipeBegin(std::uint16_t num_touches, double delta_x, double delta_y,
                                   double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGestureSwipeBegin(num_touches, delta_x, delta_y,
                                       delta_unaccel_x, delta_unaccel_y);
}

void Device::PlayGestureSwipeUpdate(std::uint16_t num_touches, double delta_x, double delta_y,
                                    double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGestureSwipeUpdate(num_touches, delta_x, delta_y,
                                        delta_unaccel_x, delta_unaccel_y);
}

void Device::PlayGestureSwipeEnd(std::uint16_t num_touches, double delta_x, double delta_y,
                                 double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGestureSwipeEnd(num_touches, delta_x, delta_y,
                                     delta_unaccel_x, delta_unaccel_y);
}

void Device::PlayGestureSwipeCancel(std::uint16_t num_touches, double delta_x, double delta_y,
                                    double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGestureSwipeCancel(num_touches, delta_x, delta_y,
                                        delta_unaccel_x, delta_unaccel_y);
}

void Device::PlayGesturePinchBegin(std::uint16_t num_touches, double delta_x, double delta_y,
                                   double delta_unaccel_x, double delta_unaccel_y,
                                   double scale, double delta_angle)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGesturePinchBegin(num_touches, delta_x, delta_y,
                                       delta_unaccel_x, delta_unaccel_y,
                                       scale, delta_angle);
}

void Device::PlayGesturePinchUpdate(std::uint16_t num_touches, double delta_x, double delta_y,
                                    double delta_unaccel_x, double delta_unaccel_y,
                                    double scale, double delta_angle)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGesturePinchUpdate(num_touches, delta_x, delta_y,
                                        delta_unaccel_x, delta_unaccel_y,
                                        scale, delta_angle);
}

void Device::PlayGesturePinchEnd(std::uint16_t num_touches, double delta_x, double delta_y,
                                 double delta_unaccel_x, double delta_unaccel_y,
                                 double scale, double delta_angle)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGesturePinchEnd(num_touches, delta_x, delta_y,
                                     delta_unaccel_x, delta_unaccel_y,
                                     scale, delta_angle);
}

void Device::PlayGesturePinchCancel(std::uint16_t num_touches, double delta_x, double delta_y,
                                    double delta_unaccel_x, double delta_unaccel_y,
                                    double scale, double delta_angle)
{
  EnsureDeviceTypeForEvent(d_->device_type, {DeviceType::POINTER_GESTURE});
  d_->connection.PlayGesturePinchCancel(num_touches, delta_x, delta_y,
                                        delta_unaccel_x, delta_unaccel_y,
                                        scale, delta_angle);
}

} // namespace xorg
} // namespace testing
} // namespace emulated
