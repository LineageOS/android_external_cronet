//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Copyright 2018 Ulf Adams
// Copyright (c) Microsoft Corporation. All rights reserved.

// Boost Software License - Version 1.0 - August 17th, 2003

// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// Avoid formatting to keep the changes with the original code minimal.
// clang-format off

#include <__assert>
#include <__config>
#include <charconv>

#include "include/ryu/common.h"
#include "include/ryu/d2fixed.h"
#include "include/ryu/d2s_intrinsics.h"
#include "include/ryu/digit_table.h"
#include "include/ryu/f2s.h"
#include "include/ryu/ryu.h"

_LIBCPP_BEGIN_NAMESPACE_STD

inline constexpr int __FLOAT_MANTISSA_BITS = 23;
inline constexpr int __FLOAT_EXPONENT_BITS = 8;
inline constexpr int __FLOAT_BIAS = 127;

inline constexpr int __FLOAT_POW5_INV_BITCOUNT = 59;
inline constexpr uint64_t __FLOAT_POW5_INV_SPLIT[31] = {
  576460752303423489u, 461168601842738791u, 368934881474191033u, 295147905179352826u,
  472236648286964522u, 377789318629571618u, 302231454903657294u, 483570327845851670u,
  386856262276681336u, 309485009821345069u, 495176015714152110u, 396140812571321688u,
  316912650057057351u, 507060240091291761u, 405648192073033409u, 324518553658426727u,
  519229685853482763u, 415383748682786211u, 332306998946228969u, 531691198313966350u,
  425352958651173080u, 340282366920938464u, 544451787073501542u, 435561429658801234u,
  348449143727040987u, 557518629963265579u, 446014903970612463u, 356811923176489971u,
  570899077082383953u, 456719261665907162u, 365375409332725730u
};
inline constexpr int __FLOAT_POW5_BITCOUNT = 61;
inline constexpr uint64_t __FLOAT_POW5_SPLIT[47] = {
  1152921504606846976u, 1441151880758558720u, 1801439850948198400u, 2251799813685248000u,
  1407374883553280000u, 1759218604441600000u, 2199023255552000000u, 1374389534720000000u,
  1717986918400000000u, 2147483648000000000u, 1342177280000000000u, 1677721600000000000u,
  2097152000000000000u, 1310720000000000000u, 1638400000000000000u, 2048000000000000000u,
  1280000000000000000u, 1600000000000000000u, 2000000000000000000u, 1250000000000000000u,
  1562500000000000000u, 1953125000000000000u, 1220703125000000000u, 1525878906250000000u,
  1907348632812500000u, 1192092895507812500u, 1490116119384765625u, 1862645149230957031u,
  1164153218269348144u, 1455191522836685180u, 1818989403545856475u, 2273736754432320594u,
  1421085471520200371u, 1776356839400250464u, 2220446049250313080u, 1387778780781445675u,
  1734723475976807094u, 2168404344971008868u, 1355252715606880542u, 1694065894508600678u,
  2117582368135750847u, 1323488980084844279u, 1654361225106055349u, 2067951531382569187u,
  1292469707114105741u, 1615587133892632177u, 2019483917365790221u
};

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __pow5Factor(uint32_t __value) {
  uint32_t __count = 0;
  for (;;) {
    _LIBCPP_ASSERT(__value != 0, "");
    const uint32_t __q = __value / 5;
    const uint32_t __r = __value % 5;
    if (__r != 0) {
      break;
    }
    __value = __q;
    ++__count;
  }
  return __count;
}

// Returns true if __value is divisible by 5^__p.
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline bool __multipleOfPowerOf5(const uint32_t __value, const uint32_t __p) {
  return __pow5Factor(__value) >= __p;
}

// Returns true if __value is divisible by 2^__p.
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline bool __multipleOfPowerOf2(const uint32_t __value, const uint32_t __p) {
  _LIBCPP_ASSERT(__value != 0, "");
  _LIBCPP_ASSERT(__p < 32, "");
  // __builtin_ctz doesn't appear to be faster here.
  return (__value & ((1u << __p) - 1)) == 0;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __mulShift(const uint32_t __m, const uint64_t __factor, const int32_t __shift) {
  _LIBCPP_ASSERT(__shift > 32, "");

  // The casts here help MSVC to avoid calls to the __allmul library
  // function.
  const uint32_t __factorLo = static_cast<uint32_t>(__factor);
  const uint32_t __factorHi = static_cast<uint32_t>(__factor >> 32);
  const uint64_t __bits0 = static_cast<uint64_t>(__m) * __factorLo;
  const uint64_t __bits1 = static_cast<uint64_t>(__m) * __factorHi;

#ifndef _LIBCPP_64_BIT
  // On 32-bit platforms we can avoid a 64-bit shift-right since we only
  // need the upper 32 bits of the result and the shift value is > 32.
  const uint32_t __bits0Hi = static_cast<uint32_t>(__bits0 >> 32);
  uint32_t __bits1Lo = static_cast<uint32_t>(__bits1);
  uint32_t __bits1Hi = static_cast<uint32_t>(__bits1 >> 32);
  __bits1Lo += __bits0Hi;
  __bits1Hi += (__bits1Lo < __bits0Hi);
  const int32_t __s = __shift - 32;
  return (__bits1Hi << (32 - __s)) | (__bits1Lo >> __s);
#else // ^^^ 32-bit ^^^ / vvv 64-bit vvv
  const uint64_t __sum = (__bits0 >> 32) + __bits1;
  const uint64_t __shiftedSum = __sum >> (__shift - 32);
  _LIBCPP_ASSERT(__shiftedSum <= UINT32_MAX, "");
  return static_cast<uint32_t>(__shiftedSum);
#endif // ^^^ 64-bit ^^^
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __mulPow5InvDivPow2(const uint32_t __m, const uint32_t __q, const int32_t __j) {
  return __mulShift(__m, __FLOAT_POW5_INV_SPLIT[__q], __j);
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __mulPow5divPow2(const uint32_t __m, const uint32_t __i, const int32_t __j) {
  return __mulShift(__m, __FLOAT_POW5_SPLIT[__i], __j);
}

// A floating decimal representing m * 10^e.
struct __floating_decimal_32 {
  uint32_t __mantissa;
  int32_t __exponent;
};

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline __floating_decimal_32 __f2d(const uint32_t __ieeeMantissa, const uint32_t __ieeeExponent) {
  int32_t __e2;
  uint32_t __m2;
  if (__ieeeExponent == 0) {
    // We subtract 2 so that the bounds computation has 2 additional bits.
    __e2 = 1 - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS - 2;
    __m2 = __ieeeMantissa;
  } else {
    __e2 = static_cast<int32_t>(__ieeeExponent) - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS - 2;
    __m2 = (1u << __FLOAT_MANTISSA_BITS) | __ieeeMantissa;
  }
  const bool __even = (__m2 & 1) == 0;
  const bool __acceptBounds = __even;

  // Step 2: Determine the interval of valid decimal representations.
  const uint32_t __mv = 4 * __m2;
  const uint32_t __mp = 4 * __m2 + 2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t __mmShift = __ieeeMantissa != 0 || __ieeeExponent <= 1;
  const uint32_t __mm = 4 * __m2 - 1 - __mmShift;

  // Step 3: Convert to a decimal power base using 64-bit arithmetic.
  uint32_t __vr, __vp, __vm;
  int32_t __e10;
  bool __vmIsTrailingZeros = false;
  bool __vrIsTrailingZeros = false;
  uint8_t __lastRemovedDigit = 0;
  if (__e2 >= 0) {
    const uint32_t __q = __log10Pow2(__e2);
    __e10 = static_cast<int32_t>(__q);
    const int32_t __k = __FLOAT_POW5_INV_BITCOUNT + __pow5bits(static_cast<int32_t>(__q)) - 1;
    const int32_t __i = -__e2 + static_cast<int32_t>(__q) + __k;
    __vr = __mulPow5InvDivPow2(__mv, __q, __i);
    __vp = __mulPow5InvDivPow2(__mp, __q, __i);
    __vm = __mulPow5InvDivPow2(__mm, __q, __i);
    if (__q != 0 && (__vp - 1) / 10 <= __vm / 10) {
      // We need to know one removed digit even if we are not going to loop below. We could use
      // __q = X - 1 above, except that would require 33 bits for the result, and we've found that
      // 32-bit arithmetic is faster even on 64-bit machines.
      const int32_t __l = __FLOAT_POW5_INV_BITCOUNT + __pow5bits(static_cast<int32_t>(__q - 1)) - 1;
      __lastRemovedDigit = static_cast<uint8_t>(__mulPow5InvDivPow2(__mv, __q - 1,
        -__e2 + static_cast<int32_t>(__q) - 1 + __l) % 10);
    }
    if (__q <= 9) {
      // The largest power of 5 that fits in 24 bits is 5^10, but __q <= 9 seems to be safe as well.
      // Only one of __mp, __mv, and __mm can be a multiple of 5, if any.
      if (__mv % 5 == 0) {
        __vrIsTrailingZeros = __multipleOfPowerOf5(__mv, __q);
      } else if (__acceptBounds) {
        __vmIsTrailingZeros = __multipleOfPowerOf5(__mm, __q);
      } else {
        __vp -= __multipleOfPowerOf5(__mp, __q);
      }
    }
  } else {
    const uint32_t __q = __log10Pow5(-__e2);
    __e10 = static_cast<int32_t>(__q) + __e2;
    const int32_t __i = -__e2 - static_cast<int32_t>(__q);
    const int32_t __k = __pow5bits(__i) - __FLOAT_POW5_BITCOUNT;
    int32_t __j = static_cast<int32_t>(__q) - __k;
    __vr = __mulPow5divPow2(__mv, static_cast<uint32_t>(__i), __j);
    __vp = __mulPow5divPow2(__mp, static_cast<uint32_t>(__i), __j);
    __vm = __mulPow5divPow2(__mm, static_cast<uint32_t>(__i), __j);
    if (__q != 0 && (__vp - 1) / 10 <= __vm / 10) {
      __j = static_cast<int32_t>(__q) - 1 - (__pow5bits(__i + 1) - __FLOAT_POW5_BITCOUNT);
      __lastRemovedDigit = static_cast<uint8_t>(__mulPow5divPow2(__mv, static_cast<uint32_t>(__i + 1), __j) % 10);
    }
    if (__q <= 1) {
      // {__vr,__vp,__vm} is trailing zeros if {__mv,__mp,__mm} has at least __q trailing 0 bits.
      // __mv = 4 * __m2, so it always has at least two trailing 0 bits.
      __vrIsTrailingZeros = true;
      if (__acceptBounds) {
        // __mm = __mv - 1 - __mmShift, so it has 1 trailing 0 bit iff __mmShift == 1.
        __vmIsTrailingZeros = __mmShift == 1;
      } else {
        // __mp = __mv + 2, so it always has at least one trailing 0 bit.
        --__vp;
      }
    } else if (__q < 31) { // TRANSITION(ulfjack): Use a tighter bound here.
      __vrIsTrailingZeros = __multipleOfPowerOf2(__mv, __q - 1);
    }
  }

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int32_t __removed = 0;
  uint32_t _Output;
  if (__vmIsTrailingZeros || __vrIsTrailingZeros) {
    // General case, which happens rarely (~4.0%).
    while (__vp / 10 > __vm / 10) {
#ifdef __clang__ // TRANSITION, LLVM-23106
      __vmIsTrailingZeros &= __vm - (__vm / 10) * 10 == 0;
#else
      __vmIsTrailingZeros &= __vm % 10 == 0;
#endif
      __vrIsTrailingZeros &= __lastRemovedDigit == 0;
      __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
      __vr /= 10;
      __vp /= 10;
      __vm /= 10;
      ++__removed;
    }
    if (__vmIsTrailingZeros) {
      while (__vm % 10 == 0) {
        __vrIsTrailingZeros &= __lastRemovedDigit == 0;
        __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
        __vr /= 10;
        __vp /= 10;
        __vm /= 10;
        ++__removed;
      }
    }
    if (__vrIsTrailingZeros && __lastRemovedDigit == 5 && __vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      __lastRemovedDigit = 4;
    }
    // We need to take __vr + 1 if __vr is outside bounds or we need to round up.
    _Output = __vr + ((__vr == __vm && (!__acceptBounds || !__vmIsTrailingZeros)) || __lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~96.0%). Percentages below are relative to this.
    // Loop iterations below (approximately):
    // 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
    while (__vp / 10 > __vm / 10) {
      __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
      __vr /= 10;
      __vp /= 10;
      __vm /= 10;
      ++__removed;
    }
    // We need to take __vr + 1 if __vr is outside bounds or we need to round up.
    _Output = __vr + (__vr == __vm || __lastRemovedDigit >= 5);
  }
  const int32_t __exp = __e10 + __removed;

  __floating_decimal_32 __fd;
  __fd.__exponent = __exp;
  __fd.__mantissa = _Output;
  return __fd;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline to_chars_result _Large_integer_to_chars(char* const _First, char* const _Last,
  const uint32_t _Mantissa2, const int32_t _Exponent2) {

  // Print the integer _Mantissa2 * 2^_Exponent2 exactly.

  // For nonzero integers, _Exponent2 >= -23. (The minimum value occurs when _Mantissa2 * 2^_Exponent2 is 1.
  // In that case, _Mantissa2 is the implicit 1 bit followed by 23 zeros, so _Exponent2 is -23 to shift away
  // the zeros.) The dense range of exactly representable integers has negative or zero exponents
  // (as positive exponents make the range non-dense). For that dense range, Ryu will always be used:
  // every digit is necessary to uniquely identify the value, so Ryu must print them all.

  // Positive exponents are the non-dense range of exactly representable integers.
  // This contains all of the values for which Ryu can't be used (and a few Ryu-friendly values).

  // Performance note: Long division appears to be faster than losslessly widening float to double and calling
  // __d2fixed_buffered_n(). If __f2fixed_buffered_n() is implemented, it might be faster than long division.

  _LIBCPP_ASSERT(_Exponent2 > 0, "");
  _LIBCPP_ASSERT(_Exponent2 <= 104, ""); // because __ieeeExponent <= 254

  // Manually represent _Mantissa2 * 2^_Exponent2 as a large integer. _Mantissa2 is always 24 bits
  // (due to the implicit bit), while _Exponent2 indicates a shift of at most 104 bits.
  // 24 + 104 equals 128 equals 4 * 32, so we need exactly 4 32-bit elements.
  // We use a little-endian representation, visualized like this:

  // << left shift <<
  // most significant
  // _Data[3] _Data[2] _Data[1] _Data[0]
  //                   least significant
  //                   >> right shift >>

  constexpr uint32_t _Data_size = 4;
  uint32_t _Data[_Data_size]{};

  // _Maxidx is the index of the most significant nonzero element.
  uint32_t _Maxidx = ((24 + static_cast<uint32_t>(_Exponent2) + 31) / 32) - 1;
  _LIBCPP_ASSERT(_Maxidx < _Data_size, "");

  const uint32_t _Bit_shift = static_cast<uint32_t>(_Exponent2) % 32;
  if (_Bit_shift <= 8) { // _Mantissa2's 24 bits don't cross an element boundary
    _Data[_Maxidx] = _Mantissa2 << _Bit_shift;
  } else { // _Mantissa2's 24 bits cross an element boundary
    _Data[_Maxidx - 1] = _Mantissa2 << _Bit_shift;
    _Data[_Maxidx] = _Mantissa2 >> (32 - _Bit_shift);
  }

  // If Ryu hasn't determined the total output length, we need to buffer the digits generated from right to left
  // by long division. The largest possible float is: 340'282346638'528859811'704183484'516925440
  uint32_t _Blocks[4];
  int32_t _Filled_blocks = 0;
  // From left to right, we're going to print:
  // _Data[0] will be [1, 10] digits.
  // Then if _Filled_blocks > 0:
  // _Blocks[_Filled_blocks - 1], ..., _Blocks[0] will be 0-filled 9-digit blocks.

  if (_Maxidx != 0) { // If the integer is actually large, perform long division.
                      // Otherwise, skip to printing _Data[0].
    for (;;) {
      // Loop invariant: _Maxidx != 0 (i.e. the integer is actually large)

      const uint32_t _Most_significant_elem = _Data[_Maxidx];
      const uint32_t _Initial_remainder = _Most_significant_elem % 1000000000;
      const uint32_t _Initial_quotient = _Most_significant_elem / 1000000000;
      _Data[_Maxidx] = _Initial_quotient;
      uint64_t _Remainder = _Initial_remainder;

      // Process less significant elements.
      uint32_t _Idx = _Maxidx;
      do {
        --_Idx; // Initially, _Remainder is at most 10^9 - 1.

        // Now, _Remainder is at most (10^9 - 1) * 2^32 + 2^32 - 1, simplified to 10^9 * 2^32 - 1.
        _Remainder = (_Remainder << 32) | _Data[_Idx];

        // floor((10^9 * 2^32 - 1) / 10^9) == 2^32 - 1, so uint32_t _Quotient is lossless.
        const uint32_t _Quotient = static_cast<uint32_t>(__div1e9(_Remainder));

        // _Remainder is at most 10^9 - 1 again.
        // For uint32_t truncation, see the __mod1e9() comment in d2s_intrinsics.h.
        _Remainder = static_cast<uint32_t>(_Remainder) - 1000000000u * _Quotient;

        _Data[_Idx] = _Quotient;
      } while (_Idx != 0);

      // Store a 0-filled 9-digit block.
      _Blocks[_Filled_blocks++] = static_cast<uint32_t>(_Remainder);

      if (_Initial_quotient == 0) { // Is the large integer shrinking?
        --_Maxidx; // log2(10^9) is 29.9, so we can't shrink by more than one element.
        if (_Maxidx == 0) {
          break; // We've finished long division. Now we need to print _Data[0].
        }
      }
    }
  }

  _LIBCPP_ASSERT(_Data[0] != 0, "");
  for (uint32_t _Idx = 1; _Idx < _Data_size; ++_Idx) {
    _LIBCPP_ASSERT(_Data[_Idx] == 0, "");
  }

  const uint32_t _Data_olength = _Data[0] >= 1000000000 ? 10 : __decimalLength9(_Data[0]);
  const uint32_t _Total_fixed_length = _Data_olength + 9 * _Filled_blocks;

  if (_Last - _First < static_cast<ptrdiff_t>(_Total_fixed_length)) {
    return { _Last, errc::value_too_large };
  }

  char* _Result = _First;

  // Print _Data[0]. While it's up to 10 digits,
  // which is more than Ryu generates, the code below can handle this.
  __append_n_digits(_Data_olength, _Data[0], _Result);
  _Result += _Data_olength;

  // Print 0-filled 9-digit blocks.
  for (int32_t _Idx = _Filled_blocks - 1; _Idx >= 0; --_Idx) {
    __append_nine_digits(_Blocks[_Idx], _Result);
    _Result += 9;
  }

  return { _Result, errc{} };
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline to_chars_result __to_chars(char* const _First, char* const _Last, const __floating_decimal_32 __v,
  chars_format _Fmt, const uint32_t __ieeeMantissa, const uint32_t __ieeeExponent) {
  // Step 5: Print the decimal representation.
  uint32_t _Output = __v.__mantissa;
  int32_t _Ryu_exponent = __v.__exponent;
  const uint32_t __olength = __decimalLength9(_Output);
  int32_t _Scientific_exponent = _Ryu_exponent + static_cast<int32_t>(__olength) - 1;

  if (_Fmt == chars_format{}) {
    int32_t _Lower;
    int32_t _Upper;

    if (__olength == 1) {
      // Value | Fixed   | Scientific
      // 1e-3  | "0.001" | "1e-03"
      // 1e4   | "10000" | "1e+04"
      _Lower = -3;
      _Upper = 4;
    } else {
      // Value   | Fixed       | Scientific
      // 1234e-7 | "0.0001234" | "1.234e-04"
      // 1234e5  | "123400000" | "1.234e+08"
      _Lower = -static_cast<int32_t>(__olength + 3);
      _Upper = 5;
    }

    if (_Lower <= _Ryu_exponent && _Ryu_exponent <= _Upper) {
      _Fmt = chars_format::fixed;
    } else {
      _Fmt = chars_format::scientific;
    }
  } else if (_Fmt == chars_format::general) {
    // C11 7.21.6.1 "The fprintf function"/8:
    // "Let P equal [...] 6 if the precision is omitted [...].
    // Then, if a conversion with style E would have an exponent of X:
    // - if P > X >= -4, the conversion is with style f [...].
    // - otherwise, the conversion is with style e [...]."
    if (-4 <= _Scientific_exponent && _Scientific_exponent < 6) {
      _Fmt = chars_format::fixed;
    } else {
      _Fmt = chars_format::scientific;
    }
  }

  if (_Fmt == chars_format::fixed) {
    // Example: _Output == 1729, __olength == 4

    // _Ryu_exponent | Printed  | _Whole_digits | _Total_fixed_length  | Notes
    // --------------|----------|---------------|----------------------|---------------------------------------
    //             2 | 172900   |  6            | _Whole_digits        | Ryu can't be used for printing
    //             1 | 17290    |  5            | (sometimes adjusted) | when the trimmed digits are nonzero.
    // --------------|----------|---------------|----------------------|---------------------------------------
    //             0 | 1729     |  4            | _Whole_digits        | Unified length cases.
    // --------------|----------|---------------|----------------------|---------------------------------------
    //            -1 | 172.9    |  3            | __olength + 1        | This case can't happen for
    //            -2 | 17.29    |  2            |                      | __olength == 1, but no additional
    //            -3 | 1.729    |  1            |                      | code is needed to avoid it.
    // --------------|----------|---------------|----------------------|---------------------------------------
    //            -4 | 0.1729   |  0            | 2 - _Ryu_exponent    | C11 7.21.6.1 "The fprintf function"/8:
    //            -5 | 0.01729  | -1            |                      | "If a decimal-point character appears,
    //            -6 | 0.001729 | -2            |                      | at least one digit appears before it."

    const int32_t _Whole_digits = static_cast<int32_t>(__olength) + _Ryu_exponent;

    uint32_t _Total_fixed_length;
    if (_Ryu_exponent >= 0) { // cases "172900" and "1729"
      _Total_fixed_length = static_cast<uint32_t>(_Whole_digits);
      if (_Output == 1) {
        // Rounding can affect the number of digits.
        // For example, 1e11f is exactly "99999997952" which is 11 digits instead of 12.
        // We can use a lookup table to detect this and adjust the total length.
        static constexpr uint8_t _Adjustment[39] = {
          0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,0,1,1,0,1,1,1 };
        _Total_fixed_length -= _Adjustment[_Ryu_exponent];
        // _Whole_digits doesn't need to be adjusted because these cases won't refer to it later.
      }
    } else if (_Whole_digits > 0) { // case "17.29"
      _Total_fixed_length = __olength + 1;
    } else { // case "0.001729"
      _Total_fixed_length = static_cast<uint32_t>(2 - _Ryu_exponent);
    }

    if (_Last - _First < static_cast<ptrdiff_t>(_Total_fixed_length)) {
      return { _Last, errc::value_too_large };
    }

    char* _Mid;
    if (_Ryu_exponent > 0) { // case "172900"
      bool _Can_use_ryu;

      if (_Ryu_exponent > 10) { // 10^10 is the largest power of 10 that's exactly representable as a float.
        _Can_use_ryu = false;
      } else {
        // Ryu generated X: __v.__mantissa * 10^_Ryu_exponent
        // __v.__mantissa == 2^_Trailing_zero_bits * (__v.__mantissa >> _Trailing_zero_bits)
        // 10^_Ryu_exponent == 2^_Ryu_exponent * 5^_Ryu_exponent

        // _Trailing_zero_bits is [0, 29] (aside: because 2^29 is the largest power of 2
        // with 9 decimal digits, which is float's round-trip limit.)
        // _Ryu_exponent is [1, 10].
        // Normalization adds [2, 23] (aside: at least 2 because the pre-normalized mantissa is at least 5).
        // This adds up to [3, 62], which is well below float's maximum binary exponent 127.

        // Therefore, we just need to consider (__v.__mantissa >> _Trailing_zero_bits) * 5^_Ryu_exponent.

        // If that product would exceed 24 bits, then X can't be exactly represented as a float.
        // (That's not a problem for round-tripping, because X is close enough to the original float,
        // but X isn't mathematically equal to the original float.) This requires a high-precision fallback.

        // If the product is 24 bits or smaller, then X can be exactly represented as a float (and we don't
        // need to re-synthesize it; the original float must have been X, because Ryu wouldn't produce the
        // same output for two different floats X and Y). This allows Ryu's output to be used (zero-filled).

        // (2^24 - 1) / 5^0 (for indexing), (2^24 - 1) / 5^1, ..., (2^24 - 1) / 5^10
        static constexpr uint32_t _Max_shifted_mantissa[11] = {
          16777215, 3355443, 671088, 134217, 26843, 5368, 1073, 214, 42, 8, 1 };

        unsigned long _Trailing_zero_bits;
        (void) _BitScanForward(&_Trailing_zero_bits, __v.__mantissa); // __v.__mantissa is guaranteed nonzero
        const uint32_t _Shifted_mantissa = __v.__mantissa >> _Trailing_zero_bits;
        _Can_use_ryu = _Shifted_mantissa <= _Max_shifted_mantissa[_Ryu_exponent];
      }

      if (!_Can_use_ryu) {
        const uint32_t _Mantissa2 = __ieeeMantissa | (1u << __FLOAT_MANTISSA_BITS); // restore implicit bit
        const int32_t _Exponent2 = static_cast<int32_t>(__ieeeExponent)
          - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS; // bias and normalization

        // Performance note: We've already called Ryu, so this will redundantly perform buffering and bounds checking.
        return _Large_integer_to_chars(_First, _Last, _Mantissa2, _Exponent2);
      }

      // _Can_use_ryu
      // Print the decimal digits, left-aligned within [_First, _First + _Total_fixed_length).
      _Mid = _First + __olength;
    } else { // cases "1729", "17.29", and "0.001729"
      // Print the decimal digits, right-aligned within [_First, _First + _Total_fixed_length).
      _Mid = _First + _Total_fixed_length;
    }

    while (_Output >= 10000) {
#ifdef __clang__ // TRANSITION, LLVM-38217
      const uint32_t __c = _Output - 10000 * (_Output / 10000);
#else
      const uint32_t __c = _Output % 10000;
#endif
      _Output /= 10000;
      const uint32_t __c0 = (__c % 100) << 1;
      const uint32_t __c1 = (__c / 100) << 1;
      std::memcpy(_Mid -= 2, __DIGIT_TABLE + __c0, 2);
      std::memcpy(_Mid -= 2, __DIGIT_TABLE + __c1, 2);
    }
    if (_Output >= 100) {
      const uint32_t __c = (_Output % 100) << 1;
      _Output /= 100;
      std::memcpy(_Mid -= 2, __DIGIT_TABLE + __c, 2);
    }
    if (_Output >= 10) {
      const uint32_t __c = _Output << 1;
      std::memcpy(_Mid -= 2, __DIGIT_TABLE + __c, 2);
    } else {
      *--_Mid = static_cast<char>('0' + _Output);
    }

    if (_Ryu_exponent > 0) { // case "172900" with _Can_use_ryu
      // Performance note: it might be more efficient to do this immediately after setting _Mid.
      std::memset(_First + __olength, '0', static_cast<size_t>(_Ryu_exponent));
    } else if (_Ryu_exponent == 0) { // case "1729"
      // Done!
    } else if (_Whole_digits > 0) { // case "17.29"
      // Performance note: moving digits might not be optimal.
      std::memmove(_First, _First + 1, static_cast<size_t>(_Whole_digits));
      _First[_Whole_digits] = '.';
    } else { // case "0.001729"
      // Performance note: a larger memset() followed by overwriting '.' might be more efficient.
      _First[0] = '0';
      _First[1] = '.';
      std::memset(_First + 2, '0', static_cast<size_t>(-_Whole_digits));
    }

    return { _First + _Total_fixed_length, errc{} };
  }

  const uint32_t _Total_scientific_length =
    __olength + (__olength > 1) + 4; // digits + possible decimal point + scientific exponent
  if (_Last - _First < static_cast<ptrdiff_t>(_Total_scientific_length)) {
    return { _Last, errc::value_too_large };
  }
  char* const __result = _First;

  // Print the decimal digits.
  uint32_t __i = 0;
  while (_Output >= 10000) {
#ifdef __clang__ // TRANSITION, LLVM-38217
    const uint32_t __c = _Output - 10000 * (_Output / 10000);
#else
    const uint32_t __c = _Output % 10000;
#endif
    _Output /= 10000;
    const uint32_t __c0 = (__c % 100) << 1;
    const uint32_t __c1 = (__c / 100) << 1;
    std::memcpy(__result + __olength - __i - 1, __DIGIT_TABLE + __c0, 2);
    std::memcpy(__result + __olength - __i - 3, __DIGIT_TABLE + __c1, 2);
    __i += 4;
  }
  if (_Output >= 100) {
    const uint32_t __c = (_Output % 100) << 1;
    _Output /= 100;
    std::memcpy(__result + __olength - __i - 1, __DIGIT_TABLE + __c, 2);
    __i += 2;
  }
  if (_Output >= 10) {
    const uint32_t __c = _Output << 1;
    // We can't use memcpy here: the decimal dot goes between these two digits.
    __result[2] = __DIGIT_TABLE[__c + 1];
    __result[0] = __DIGIT_TABLE[__c];
  } else {
    __result[0] = static_cast<char>('0' + _Output);
  }

  // Print decimal point if needed.
  uint32_t __index;
  if (__olength > 1) {
    __result[1] = '.';
    __index = __olength + 1;
  } else {
    __index = 1;
  }

  // Print the exponent.
  __result[__index++] = 'e';
  if (_Scientific_exponent < 0) {
    __result[__index++] = '-';
    _Scientific_exponent = -_Scientific_exponent;
  } else {
    __result[__index++] = '+';
  }

  std::memcpy(__result + __index, __DIGIT_TABLE + 2 * _Scientific_exponent, 2);
  __index += 2;

  return { _First + _Total_scientific_length, errc{} };
}

[[nodiscard]] to_chars_result __f2s_buffered_n(char* const _First, char* const _Last, const float __f,
  const chars_format _Fmt) {

  // Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
  const uint32_t __bits = __float_to_bits(__f);

  // Case distinction; exit early for the easy cases.
  if (__bits == 0) {
    if (_Fmt == chars_format::scientific) {
      if (_Last - _First < 5) {
        return { _Last, errc::value_too_large };
      }

      std::memcpy(_First, "0e+00", 5);

      return { _First + 5, errc{} };
    }

    // Print "0" for chars_format::fixed, chars_format::general, and chars_format{}.
    if (_First == _Last) {
      return { _Last, errc::value_too_large };
    }

    *_First = '0';

    return { _First + 1, errc{} };
  }

  // Decode __bits into mantissa and exponent.
  const uint32_t __ieeeMantissa = __bits & ((1u << __FLOAT_MANTISSA_BITS) - 1);
  const uint32_t __ieeeExponent = __bits >> __FLOAT_MANTISSA_BITS;

  // When _Fmt == chars_format::fixed and the floating-point number is a large integer,
  // it's faster to skip Ryu and immediately print the integer exactly.
  if (_Fmt == chars_format::fixed) {
    const uint32_t _Mantissa2 = __ieeeMantissa | (1u << __FLOAT_MANTISSA_BITS); // restore implicit bit
    const int32_t _Exponent2 = static_cast<int32_t>(__ieeeExponent)
      - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS; // bias and normalization

    // Normal values are equal to _Mantissa2 * 2^_Exponent2.
    // (Subnormals are different, but they'll be rejected by the _Exponent2 test here, so they can be ignored.)

    if (_Exponent2 > 0) {
      return _Large_integer_to_chars(_First, _Last, _Mantissa2, _Exponent2);
    }
  }

  const __floating_decimal_32 __v = __f2d(__ieeeMantissa, __ieeeExponent);
  return __to_chars(_First, _Last, __v, _Fmt, __ieeeMantissa, __ieeeExponent);
}

_LIBCPP_END_NAMESPACE_STD

// clang-format on
