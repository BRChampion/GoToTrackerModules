#pragma once
#include "TrackerTypes.h"

struct Clock {
    virtual ~Clock() = default;
    virtual TickMicros micros() const = 0;
    virtual TickMicros millis() const = 0;
};
