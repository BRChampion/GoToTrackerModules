#include "AxisController.h"
#include <math.h>

AxisController::AxisController(Clock& clock, Driver& driver)
    : _clock(clock), _driver(driver) {}

// ----- Begin -----
void AxisController::begin() {
    _lastStepMicros = _clock.micros();
}

// ----- Enable -----
void AxisController::enable(bool on) {
    _enabled = on;
    _driver.enable(on);
    if (!on) {
        // Stop issuing steps when disabled
        _mode = Mode::Idle;
    } else {
        // Reset scheduling so we don't burst
        _lastStepMicros = _clock.micros();
    }
}

bool AxisController::atTarget(StepCount toleranceSteps) const {
    if (_mode != Mode::Goto) {
        // Might change later for atTarget in other modes
        return false;
    }
    StepCount err = _targetSteps - _posSteps;
    if (err < 0) err = -err;
    return err <= toleranceSteps;
}

void AxisController::startRate(RateStepsPerSec stepsPerSec) {
    _mode = Mode::Rate;
    _rateStepsPerSec = stepsPerSec;

    // Reset so we don't burst
    _lastStepMicros = _clock.micros();
}

void AxisController::startGoto(StepCount targetSteps, RateStepsPerSec maxStepsPerSec) {
    _mode = Mode::Goto;
    _targetSteps = targetSteps;
    _maxGotoStepsPerSec = maxStepsPerSec;

    // Reset so we don't burst
    _lastStepMicros = _clock.micros();
}

void AxisController::stop() {
    _mode = Mode::Idle;
}

TickMicros AxisController::intervalUsFromRate(RateStepsPerSec stepsPerSecAbs) {
    if (stepsPerSecAbs <= 0.0f) return 0;
    // 1 million micros per sec
    float interval = 1000000.0f / stepsPerSecAbs;
    if (interval < 1.0f) interval = 1.0f; // clamp to at least 1 us
    return static_cast<TickMicros>(interval);
}

bool AxisController::timeForStep(TickMicros nowUs, TickMicros intervalUs) {
   // Steady-interval scheduling
   // Advancing _lastStepMicros by intervalUs should reduce jitter
   // and avoid bursts after stalls
    if (intervalUs <= 0) return false;

    if ((TickMicros)(nowUs - _lastStepMicros) >= intervalUs) {
        _lastStepMicros += intervalUs;
        return true;
    }
    return false;
}

bool AxisController::update() {
    if (!_enabled) return false;

    const TickMicros nowUs = _clock.micros();

    if (_mode == Mode::Idle) {
        return false;
    }

    if (_mode == Mode::Rate) {
        // RATE mode: step continously at _rateStepsPerSec
        const RateStepsPerSec rate = _rateStepsPerSec;
        const RateStepsPerSec rateAbs = fabsf(rate);

        const TickMicros intervalUS = intervalUsFromRate(rateAbs);
        if (!timeForStep(nowUs, intervalUS)) return false;

        const StepDir dir = (rate >= 0.0) ? StepDir::Forward : StepDir::Backward;
        _driver.step(dir);
        _posSteps += (dir == StepDir::Forward) ? 1 : -1;
        return true;
    }

    // GOTO mode
    if (_mode == Mode::Goto) {
        // If already at target, stop
        if (_posSteps == _targetSteps) {
            _mode = Mode::Idle;
            return false;
        }

        const StepCount err = _targetSteps - _posSteps;
        const StepDir dir = (err >= 0) ? StepDir::Forward : StepDir::Backward;

        const RateStepsPerSec maxRateAbs = fabsf(_maxGotoStepsPerSec);
        const TickMicros intervalUS = intervalUsFromRate(maxRateAbs);
        if (!timeForStep(nowUs, intervalUS)) return false;

        _driver.step(dir);
        _posSteps += (dir == StepDir::Forward) ? 1 : -1;

        if (_posSteps == _targetSteps) {
            _mode = Mode::Idle;
        }
        return true;
    }
    return false;
}




