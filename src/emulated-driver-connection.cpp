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

constexpr std::int32_t XORG_KEY_CODE_OFFSET = 8;

void SendEvent(int fd, const EmulatedEvent& ev)
{
  write(fd, &ev, sizeof(ev));
}

EmulatedEvent CreateEvent(EmulatedEventType type)
{
  EmulatedEvent event;
  std::memset(&event, 0, sizeof(event));
  event.any.event = type;
  return event;
}

void SendButtonEvent(int fd, bool is_down, bool is_absolute, std::int32_t button,
                     const Valuators& valuators)
{
  EmulatedEvent ev = CreateEvent(EmulatedEvent_Button);
  ev.button.is_absolute = is_absolute;
  ev.button.button = button;
  ev.button.is_down = is_down;
  valuators.RetrieveValuatorData(&ev.button.valuators);
  SendEvent(fd, ev);
}

void SendTouchEvent(int fd, std::uint16_t type, std::uint32_t touchid, const Valuators& valuators)
{
  EmulatedEvent ev = CreateEvent(EmulatedEvent_Touch);
  ev.touch.type = type;
  ev.touch.touchid = touchid;
  ev.touch.flags = 0;
  valuators.RetrieveValuatorData(&ev.touch.valuators);
  SendEvent(fd, ev);
}

void SendGesturePinchEvent(int fd, std::uint16_t type, std::uint32_t flags,
                           std::uint16_t num_touches, double delta_x, double delta_y,
                           double delta_unaccel_x, double delta_unaccel_y,
                           double scale, double delta_angle)
{
  EmulatedEvent ev = CreateEvent(EmulatedEvent_GesturePinch);
  ev.pinch.type = type;
  ev.pinch.num_touches = num_touches;
  ev.pinch.flags = flags;
  ev.pinch.delta_x = delta_x;
  ev.pinch.delta_y = delta_y;
  ev.pinch.delta_unaccel_x = delta_unaccel_x;
  ev.pinch.delta_unaccel_y = delta_unaccel_y;
  ev.pinch.scale = scale;
  ev.pinch.delta_angle = delta_angle;
  SendEvent(fd, ev);
}

void SendGestureSwipeEvent(int fd, std::uint16_t type, std::uint32_t flags,
                           std::uint16_t num_touches, double delta_x, double delta_y,
                           double delta_unaccel_x, double delta_unaccel_y)
{
  EmulatedEvent ev = CreateEvent(EmulatedEvent_GestureSwipe);
  ev.swipe.type = type;
  ev.swipe.num_touches = num_touches;
  ev.swipe.flags = flags;
  ev.swipe.delta_x = delta_x;
  ev.swipe.delta_y = delta_y;
  ev.swipe.delta_unaccel_x = delta_unaccel_x;
  ev.swipe.delta_unaccel_y = delta_unaccel_y;
  SendEvent(fd, ev);
}

} // namespace

struct DriverConnection::Private {
  std::string events_path;
  std::string responses_path;
  int events_fd = -1;
  int responses_fd = -1;
};

DriverConnection::DriverConnection(const std::string& events_path,
                                   const std::string& responses_path) :
  d_{std::unique_ptr<Private>(new Private)}
{
  d_->events_path = events_path;
  d_->responses_path = responses_path;
}

DriverConnection::~DriverConnection()
{
  Close();
}

const std::string& DriverConnection::EventsInPath() const
{
  return d_->events_path;
}

const std::string& DriverConnection::ResponsesOutPath() const
{
  return d_->responses_path;
}

void DriverConnection::WaitOpen()
{
  unsigned timeout_ms = 10000;
  unsigned sleep_ms = 20;

  for (unsigned i = 0; i < timeout_ms; i += sleep_ms) {
    if (d_->events_fd == -1) {
      if (access(d_->events_path.c_str(), F_OK) != -1) {
        d_->events_fd = open(d_->events_path.c_str(), O_WRONLY, 0);
      }
    }
    if (d_->responses_fd == -1) {
      if (access(d_->responses_path.c_str(), F_OK) != -1) {
        d_->responses_fd = open(d_->responses_path.c_str(), O_RDONLY, 0);
      }
    }
    if (d_->events_fd != -1 && d_->responses_fd != -1)
      return;

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
  }

  throw std::runtime_error("Timed out waiting for device files to be opened");
}

void DriverConnection::Close()
{
  if (d_->events_fd != -1) {
    close(d_->events_fd);
    d_->events_fd = -1;
  }
  if (d_->responses_fd != -1) {
    close(d_->responses_fd);
    d_->responses_fd = -1;
  }

}

void DriverConnection::EnsureOpen()
{
  if (d_->events_fd == -1 || d_->responses_fd == -1)
    throw std::runtime_error("DriverConnection is not open");
}

void DriverConnection::PlayRelMotion(const Valuators& valuators)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Motion);
  ev.motion.is_absolute = false;
  valuators.RetrieveValuatorData(&ev.motion.valuators);
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayAbsMotion(const Valuators& valuators)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Motion);
  ev.motion.is_absolute = true;
  valuators.RetrieveValuatorData(&ev.motion.valuators);
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayProximityIn(const Valuators& valuators)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Proximity);
  ev.proximity.is_in = true;
  valuators.RetrieveValuatorData(&ev.proximity.valuators);
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayProximityOut(const Valuators& valuators)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Proximity);
  ev.proximity.is_in = false;
  valuators.RetrieveValuatorData(&ev.proximity.valuators);
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayButtonDownAbs(std::int32_t button, const Valuators& valuators)
{
  EnsureOpen();
  SendButtonEvent(d_->events_fd, true, true, button, valuators);
  Sync();
}

void DriverConnection::PlayButtonDownRel(std::int32_t button, const Valuators& valuators)
{
  EnsureOpen();
  SendButtonEvent(d_->events_fd, true, false, button, valuators);
  Sync();
}

void DriverConnection::PlayButtonUpAbs(std::int32_t button, const Valuators& valuators)
{
  EnsureOpen();
  SendButtonEvent(d_->events_fd, false, true, button, valuators);
  Sync();
}

void DriverConnection::PlayButtonUpRel(std::int32_t button, const Valuators& valuators)
{
  EnsureOpen();
  SendButtonEvent(d_->events_fd, false, false, button, valuators);
  Sync();
}

void DriverConnection::PlayKeyDown(std::int32_t key_code)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Key);
  ev.key.is_down = true;
  ev.key.key_code = key_code + XORG_KEY_CODE_OFFSET;
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayKeyUp(std::int32_t key_code)
{
  EnsureOpen();

  EmulatedEvent ev = CreateEvent(EmulatedEvent_Key);
  ev.key.is_down = false;
  ev.key.key_code = key_code + XORG_KEY_CODE_OFFSET;
  SendEvent(d_->events_fd, ev);
  Sync();
}

void DriverConnection::PlayTouchBegin(std::uint32_t id, const Valuators& valuators)
{
  EnsureOpen();
  SendTouchEvent(d_->events_fd, XI_TouchBegin, id, valuators);
  Sync();
}

void DriverConnection::PlayTouchUpdate(std::uint32_t id, const Valuators& valuators)
{
  EnsureOpen();
  SendTouchEvent(d_->events_fd, XI_TouchUpdate, id, valuators);
  Sync();
}

void DriverConnection::PlayTouchEnd(std::uint32_t id, const Valuators& valuators)
{
  EnsureOpen();
  SendTouchEvent(d_->events_fd, XI_TouchEnd, id, valuators);
  Sync();
}

void DriverConnection::PlayGestureSwipeBegin(std::uint16_t num_touches,
                                             double delta_x, double delta_y,
                                             double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureOpen();
  SendGestureSwipeEvent(d_->events_fd, XI_GestureSwipeBegin, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y);
  Sync();
}

void DriverConnection::PlayGestureSwipeUpdate(std::uint16_t num_touches,
                                              double delta_x, double delta_y,
                                              double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureOpen();
  SendGestureSwipeEvent(d_->events_fd, XI_GestureSwipeUpdate, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y);
  Sync();
}

void DriverConnection::PlayGestureSwipeEnd(std::uint16_t num_touches,
                                           double delta_x, double delta_y,
                                           double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureOpen();
  SendGestureSwipeEvent(d_->events_fd, XI_GestureSwipeEnd, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y);
  Sync();
}

void DriverConnection::PlayGestureSwipeCancel(std::uint16_t num_touches,
                                              double delta_x, double delta_y,
                                              double delta_unaccel_x, double delta_unaccel_y)
{
  EnsureOpen();
  SendGestureSwipeEvent(d_->events_fd, XI_GestureSwipeEnd, XIGestureSwipeEventCancelled,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y);
  Sync();
}

void DriverConnection::PlayGesturePinchBegin(std::uint16_t num_touches,
                                             double delta_x, double delta_y,
                                             double delta_unaccel_x, double delta_unaccel_y,
                                             double scale, double delta_angle)
{
  EnsureOpen();
  SendGesturePinchEvent(d_->events_fd, XI_GesturePinchBegin, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y,
                        scale, delta_angle);
  Sync();
}

void DriverConnection::PlayGesturePinchUpdate(std::uint16_t num_touches,
                                              double delta_x, double delta_y,
                                              double delta_unaccel_x, double delta_unaccel_y,
                                              double scale, double delta_angle)
{
  EnsureOpen();
  SendGesturePinchEvent(d_->events_fd, XI_GesturePinchUpdate, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y,
                        scale, delta_angle);
  Sync();
}

void DriverConnection::PlayGesturePinchEnd(std::uint16_t num_touches,
                                           double delta_x, double delta_y,
                                           double delta_unaccel_x, double delta_unaccel_y,
                                           double scale, double delta_angle)
{
  EnsureOpen();
  SendGesturePinchEvent(d_->events_fd, XI_GesturePinchEnd, 0,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y,
                        scale, delta_angle);
  Sync();
}

void DriverConnection::PlayGesturePinchCancel(std::uint16_t num_touches,
                                              double delta_x, double delta_y,
                                              double delta_unaccel_x, double delta_unaccel_y,
                                              double scale, double delta_angle)
{
  EnsureOpen();
  SendGesturePinchEvent(d_->events_fd, XI_GesturePinchEnd, XIGesturePinchEventCancelled,
                        num_touches, delta_x, delta_y, delta_unaccel_x, delta_unaccel_y,
                        scale, delta_angle);
  Sync();
}

void DriverConnection::Sync()
{
  SendEvent(d_->events_fd, CreateEvent(EmulatedEvent_WaitForSync));
  std::uint8_t sync;
  if (read(d_->responses_fd, &sync, 1) != 1) {
    throw std::runtime_error("Could not read synchronization response");
  }
  if (sync != EMULATED_SYNC_RESPONSE) {
    throw std::runtime_error("Did not read expected synchronization response");
  }
}

} // namespace xorg
} // namespace testing
} // namespace emulated
