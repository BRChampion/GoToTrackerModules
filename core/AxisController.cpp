#include "AxisController.h"
#include <cmath>    // fabs
#include <cstdint>

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

bool AxisController::atTarget(int64_t toleranceSteps) const {
    if (_mode != Mode::Goto) {
        // Might change later for atTarget in other modes
        return false;
    }
    int64_t err = _targetSteps - _posSteps;
    if (err < 0) err = -err;
    return err <= toleranceSteps;
}

void AxisController::startRate(double stepsPerSec) {
    _mode = Mode::Rate;
    _rateStepsPerSec = stepsPerSec;

    // Reset so we don't burst
    _lastStepMicros = _clock.micros();
}

void AxisController::startGoto(int64_t targetSteps, double maxStepsPerSec) {
    _mode = Mode::Goto;
    _targetSteps = targetSteps;
    _maxGotoStepsPerSec = maxStepsPerSec;

    // Reset so we don't burst
    _lastStepMicros = _clock.micros();
}

void AxisController::stop() {
    _mode = Mode::Idle;
}

uint64_t AxisController::intervalUsFromRate(double stepsPerSecAbs) {
    if (stepsPerSecAbs <= 0.0) return 0;
    // 1 million micros per sec
    double interval = 1000000.0 / stepsPerSecAbs;
    if (interval < 1.0) interval = 1.0; // clamp to at least 1 us
    return static_cast<uint64_t>(interval);
}

bool AxisController::timeForStep(uint64_t nowUs, uint64_t intervalUs) {
   // Steady-interval scheduling
   // Advancing _lastStepMicros by intervalUs should reduce jitter
   // and avoid bursts after stalls
    if (intervalUs <= 0) return false;

    if ((uint64_t)(nowUs - _lastStepMicros) >= intervalUs) {
        _lastStepMicros += intervalUs;
        return true;
    }
    return false;
}

bool AxisController::update() {
    if (!_enabled) return false;

    const int64_t nowUs = _clock.micros();

    if (_mode == Mode::Idle) {
        return false;
    }

    if (_mode == Mode::Rate) {
        // RATE mode: step continously at _rateStepsPerSec
        const double rate = _rateStepsPerSec;
        const double rateAbs = std::fabs(rate);

        const uint64_t intervalUS = intervalUsFromRate(rateAbs);
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

        const int64_t err = _targetSteps - _posSteps;
        const StepDir dir = (err >= 0) ? StepDir::Forward : StepDir::Backward;

        const double maxRateAbs = std::fabs(_maxGotoStepsPerSec);
        const uint64_t intervalUS = intervalUsFromRate(maxRateAbs);
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




