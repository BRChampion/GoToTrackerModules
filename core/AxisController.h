#pragma once

#include "Clock.h"
#include "Driver.h"
#include "TrackerTypes.h"

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

    StepCount posSteps() const {return _posSteps; }
    void setPosSteps (StepCount steps) {_posSteps  = steps; }

    StepCount targetSteps() const {return _targetSteps; }
    bool atTarget(StepCount toleranceSteps = 0) const;

    // ----- Rate mode (tracking) -----
    // Note: stepsPerSec can be fractional, sign determines direction
    void startRate(RateStepsPerSec stepsPerSec);

    // ----- Goto mode -----
    // Move toward targetSteps at (up to) maxStepsPerSec
    // Note: maxStepsPerSec MUST be > 0
    void startGoto(StepCount targetSteps, RateStepsPerSec maxStepsPerSec);

    // Stop motion
    void stop();

    // Call repeatedly
    // Return true if step was issued
    bool update();

private:
    // Schedule helper: decides if it's time for next step
    bool timeForStep(TickMicros nowUs, TickMicros intervalUs);

    // Compute interval between steps from steps/sec
    // Returns zero for invalid rate (<= 0)
    static TickMicros intervalUsFromRate(RateStepsPerSec stepsPerSecAbs);

private:
    Clock& _clock;
    Driver& _driver;

    Mode _mode = Mode::Idle;
    bool _enabled = false;

    // Unbounded axis position (in steps)
    StepCount _posSteps  = 0;

    // GOTO target
    StepCount _targetSteps = 0;

    // For RATE mode
    RateStepsPerSec _rateStepsPerSec = 0.0f;

    // For GOTO mode
    RateStepsPerSec _maxGotoStepsPerSec = 0.0f;

    // Timing state (steady interval scheduling)
    TickMicros _lastStepMicros = 0;
};
