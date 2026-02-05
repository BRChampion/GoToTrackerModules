#pragma once
#include <cstdint>

struct Clock {
    virtual ~Clock() = default;
    virtual uint64_t micros() const = 0;
    virtual uint64_t millis() const = 0;
};
