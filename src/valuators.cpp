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

#include "xorg/gtest/emulated/xorg-gtest-valuators.h"
#include <cstring>

namespace xorg {
namespace testing {
namespace emulated {

static void ValuatorSetBit(std::uint8_t* ptr, unsigned bit)
{
    ptr[bit >> 3] |= 1 << (bit & 7);
}

struct Valuators::Private {
  Private()
  {
    std::memset(&valuators, 0, sizeof(valuators));
  }

  EmulatedValuatorData valuators;
};

Valuators::Valuators() : d_{std::unique_ptr<Private>(new Private())} {}

Valuators::Valuators(const Valuators& other) : d_{std::unique_ptr<Private>(new Private())}
{
  *d_ = *other.d_;
}

Valuators& Valuators::operator=(const Valuators& other)
{
  *d_ = *other.d_;
  return *this;
}

Valuators::~Valuators() {}

Valuators& Valuators::Set(unsigned axis, double value)
{
  ValuatorSetBit(d_->valuators.mask, axis);
  d_->valuators.valuators[axis] = value;
  return *this;
}

Valuators& Valuators::SetUnaccel(unsigned axis, double value_accel, double value_unaccel)
{
  d_->valuators.has_unaccelerated = true;
  ValuatorSetBit(d_->valuators.mask, axis);
  d_->valuators.valuators[axis] = value_accel;
  d_->valuators.unaccelerated[axis] = value_unaccel;
  return *this;
}

void Valuators::RetrieveValuatorData(EmulatedValuatorData* out) const
{
  *out = d_->valuators;
}

} // namespace emulated
} // namespace testing
} // namespace xorg
