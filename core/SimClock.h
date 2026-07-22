#pragma once
#include "../src/Clock.h"

class SimClock : public Clock {
public:
    void advanceMicros(TickMicros us) { _micros += us; }
    TickMicros micros() const override { return _micros; }
    TickMicros millis() const override { return _micros / 1000UL; }
private:
    TickMicros _micros = 0;
};
