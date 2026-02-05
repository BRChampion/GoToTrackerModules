#pragma once

#include <cstdint>
#include "Clock.h"
#include "Driver.h"

// AxisController
//  - Owns control logic for ONE axis (RA/Dec)
//  - Schedules steps at steady intervals
//  - Supports two modes:
//          1) RATE : continuous stepping at fixed rate (steps/sec)
//          2) GOTO : step toward a target position at max speed (steps/sec)

// NOTE: this controller deals ONLY in steps and step rates
// and does NOT care about degrees/gearing/sky math

class AxisController {
public:
    enum class Mode {Idle, Rate, Goto};

    AxisController(Clock& clock, Driver& driver);

    void begin();

    // Enable/disable driver outputs. Note: disabling does not reset position
    void enable(bool on);


    // ----- State access -----
    Mode mode() const {return _mode; }
    bool enabled() const {return _enabled; }

    int64_t posSteps() const {return _posSteps; }
    void setPosSteps (int64_t steps) {_posSteps  = steps; }

    int64_t targetSteps() const {return _targetSteps; }
    bool atTarget(int64_t toleranceSteps = 0) const;

    // ----- Rate mode (tracking) -----
    // Note: stepsPerSec can be fractional, sign determines direction
    void startRate(double stepsPerSec);

    // ----- Goto mode -----
    // Move toward targetSteps at (up to) maxStepsPerSec
    // Note: maxStepsPerSec MUST be > 0
    void startGoto(int64_t targetSteps, double maxStepsPerSec);

    // Stop motion
    void stop();

    // Call repeatedly
    // Return true if step was issued
    bool update();

private:
    // Schedule helper: decides if it's time for next step
    bool timeForStep(uint64_t nowUs, uint64_t intervalUs);

    // Compute interval between steps from steps/sec
    // Returns zero for invalid rate (<= 0)
    static uint64_t intervalUsFromRate(double stepsPerSecAbs);

private:
    Clock& _clock;
    Driver& _driver;

    Mode _mode = Mode::Idle;
    bool _enabled = false;

    // Unbounded axis position (in steps)
    int64_t _posSteps  = 0;

    // GOTO target
    int64_t _targetSteps = 0;

    // For RATE mode
    double _rateStepsPerSec = 0.0;

    // For GOTO mode
    double _maxGotoStepsPerSec = 0.0;

    // Timing state (steady interval scheduling)
    uint64_t _lastStepMicros = 0;
};