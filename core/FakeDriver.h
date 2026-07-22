#pragma once
#include "../src/Driver.h"
#include "../src/TrackerTypes.h"

class FakeDriver : public Driver {
public:
    void enable(bool on) override { enabled = on; }
    void step(StepDir dir) override {
        if (!enabled) return;
        posSteps += (dir == StepDir::Forward) ? 1 : -1;
        totalPulses++;
    }

    bool enabled = false;
    StepCount posSteps = 0;
    uint32_t totalPulses = 0;
};
