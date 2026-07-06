#pragma once
#include <stdint.h>

enum class StepDir : int8_t { Forward = 1, Backward = -1 };

struct Driver {
    virtual ~Driver() = default;
    virtual void enable(bool on) = 0;
    virtual void step(StepDir dir) = 0;
};
