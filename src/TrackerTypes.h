#pragma once

#include <stdint.h>

// Keep these aliases AVR-friendly. On Arduino MEGA, double is also 32-bit,
// so floats make the precision tradeoff explicit and save RAM/flash.
typedef float AngleDeg;
typedef float RateStepsPerSec;

// Signed 32-bit positions are enough for many gear ratios while avoiding
// expensive 64-bit arithmetic on an 8-bit AVR.
typedef int32_t StepCount;

// Arduino micros()/millis() are 32-bit counters. Unsigned subtraction handles
// rollover correctly as long as intervals are shorter than half the range.
typedef uint32_t TickMicros;
typedef uint32_t UnixSeconds;
