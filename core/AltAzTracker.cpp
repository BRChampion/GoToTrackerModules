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

    StepCount currentWrapped = _azAxis.posSteps() % revSteps;
    if (currentWrapped < 0) currentWrapped += revSteps;

    StepCount delta = desiredAzSteps - currentWrapped;
    const StepCount halfRev = revSteps / 2;
    if (delta > halfRev) delta -= revSteps;
    if (delta < -halfRev) delta += revSteps;

    return _azAxis.posSteps() + delta;
}
