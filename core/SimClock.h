#pragma once
#include "Clock.h"

class SimClock : public Clock {
public:
    void advanceMicros(uint64_t us) { _micros += us; }
    uint64_t micros() const override { return _micros; }
    uint64_t millis() const override { return _micros / 1000; }
private:
    uint64_t _micros = 0;
};
