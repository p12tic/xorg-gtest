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

#ifndef XORG_GTEST_EMULATED_VALUATORS_H_
#define XORG_GTEST_EMULATED_VALUATORS_H_

#include <memory>
#include <string>

#include <xorg/emulated-events.h>

namespace xorg {
namespace testing {
namespace emulated {

/**
 * A class that represents the valuator values sent to the X server.
 * The axes are hardcoded according to the current implementation of the xf86-input-emulated
 * input driver.
 */

class Valuators {
 public:
  Valuators();
  Valuators(const Valuators& other);
  Valuators& operator=(const Valuators& other);
  ~Valuators();

  Valuators& Set(unsigned axis, double value);
  Valuators& SetUnaccel(unsigned axis, double value_accel, double value_unaccel);

  void RetrieveValuatorData(EmulatedValuatorData* out) const;

 private:
  struct Private;
  std::unique_ptr<Private> d_;
};

} // namespace emulated
} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_EMULATED_VALUATORS_H_
