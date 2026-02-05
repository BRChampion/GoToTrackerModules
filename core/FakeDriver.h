#pragma once
#include "Driver.h"
#include <cstdint>

class FakeDriver : public Driver {
public:
    void enable(bool on) override { enabled = on; }
    void step(StepDir dir) override {
        if (!enabled) return;
        posSteps += (dir == StepDir::Forward) ? 1 : -1;
        totalPulses++;
    }

    bool enabled = false;
    int64_t posSteps = 0;
    uint64_t totalPulses = 0;
};
