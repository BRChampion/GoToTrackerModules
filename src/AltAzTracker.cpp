#include "AltAzTracker.h"

AltAzTracker::AltAzTracker(Clock& clock,
                           AxisController& altAxis,
                           AxisController& azAxis,
                           const MountModel& altModel,
                           const MountModel& azModel)
    : _clock(clock),
      _altAxis(altAxis),
      _azAxis(azAxis),
      _altModel(altModel),
      _azModel(azModel) {}

void AltAzTracker::begin() {
    _epochMicros = _clock.micros();
    _lastRefreshMicros = _epochMicros;
}

void AltAzTracker::setObserver(AngleDeg latDeg, AngleDeg lonDeg) {
    _latDeg = latDeg;
    _lonDeg = lonDeg;
}

void AltAzTracker::setTime(UnixSeconds unixSeconds) {
    _epochUnix = unixSeconds;
    _epochMicros = _clock.micros();
    _lastRefreshMicros = _epochMicros;
}

void AltAzTracker::setTarget(SkyMath::EquatorialCoord target) {
    _target = target;
    _hasTarget = true;
    refreshTargetSteps();
}

void AltAzTracker::manualHome() {
    // Manual home means the operator has physically placed the mount at the
    // chosen Alt/Az zero reference, so software can reset both step counters.
    _tracking = false;
    _altAxis.stop();
    _azAxis.stop();
    _altAxis.setPosSteps(0);
    _azAxis.setPosSteps(0);
    _homed = true;
    _calibrated = false;
    refreshTargetSteps();
}

bool AltAzTracker::syncOneStar() {
    if (!_hasTarget) return false;

    // After the operator centers the selected star, align the software step
    // position with where that star should be at the current time/location.
    refreshTargetSteps();
    _tracking = false;
    _altAxis.stop();
    _azAxis.stop();
    _altAxis.setPosSteps(_altTargetSteps);
    _azAxis.setPosSteps(_azTargetSteps);
    _calibrated = true;
    return true;
}

void AltAzTracker::startGoto() {
    if (!_hasTarget) return;
    refreshTargetSteps();
    _altAxis.startGoto(_altTargetSteps, _gotoRateStepsPerSec);
    _azAxis.startGoto(_azTargetSteps, _gotoRateStepsPerSec);
}

void AltAzTracker::setTracking(bool on) {
    _tracking = on && _hasTarget;
    if (_tracking) {
        refreshTargetSteps();
        startGoto();
    }
}

void AltAzTracker::stop() {
    _tracking = false;
    _altAxis.stop();
    _azAxis.stop();
}

void AltAzTracker::update() {
    const TickMicros now = _clock.micros();

    // Alt/Az tracking changes both axes over time. Refresh the desired target
    // periodically, then let each AxisController step toward it incrementally.
    if (_tracking && (TickMicros)(now - _lastRefreshMicros) >= _refreshIntervalMicros) {
        _lastRefreshMicros += _refreshIntervalMicros;
        refreshTargetSteps();

        if (_altAxis.targetSteps() != _altTargetSteps) {
            _altAxis.startGoto(_altTargetSteps, _gotoRateStepsPerSec);
        }
        if (_azAxis.targetSteps() != _azTargetSteps) {
            _azAxis.startGoto(_azTargetSteps, _gotoRateStepsPerSec);
        }
    }

    _altAxis.update();
    _azAxis.update();
}

UnixSeconds AltAzTracker::unixNow() const {
    // Use elapsed micros since the last time set instead of continuously
    // mutating epoch time; unsigned subtraction keeps micros rollover safe.
    return _epochUnix + (UnixSeconds)((TickMicros)(_clock.micros() - _epochMicros) / 1000000UL);
}

void AltAzTracker::refreshTargetSteps() {
    if (!_hasTarget) return;

    const AngleDeg lst = SkyMath::lstDegFromUnix(unixNow(), _lonDeg);
    _lastHorizontal = SkyMath::equatorialToHorizontal(_target, lst, _latDeg);

    _altTargetSteps = _altModel.degToSteps(_lastHorizontal.altDeg);
    _azTargetSteps = nearestWrappedAzTarget(_azModel.degToSteps(_lastHorizontal.azDeg));
}

StepCount AltAzTracker::nearestWrappedAzTarget(StepCount desiredAzSteps) const {
    const StepCount revSteps = _azModel.stepsPerAxisRevRounded();
    if (revSteps <= 0) return desiredAzSteps;

    // Azimuth is circular. Convert the requested azimuth to the nearest
    // equivalent unbounded step position so 359 -> 1 degrees moves +2, not -358.
    StepCount currentWrapped = _azAxis.posSteps() % revSteps;
    if (currentWrapped < 0) currentWrapped += revSteps;

    StepCount delta = desiredAzSteps - currentWrapped;
    const StepCount halfRev = revSteps / 2;
    if (delta > halfRev) delta -= revSteps;
    if (delta < -halfRev) delta += revSteps;

    return _azAxis.posSteps() + delta;
}
