#pragma once

#include "AxisController.h"
#include "MountModel.h"
#include "SkyMath.h"

class AltAzTracker {
public:
    AltAzTracker(Clock& clock,
                 AxisController& altAxis,
                 AxisController& azAxis,
                 const MountModel& altModel,
                 const MountModel& azModel);

    void begin();
    void setObserver(AngleDeg latDeg, AngleDeg lonDeg);
    void setTime(UnixSeconds unixSeconds);
    void setTarget(SkyMath::EquatorialCoord target);

    bool hasTarget() const { return _hasTarget; }
    bool tracking() const { return _tracking; }

    void startGoto();
    void setTracking(bool on);
    void stop();

    void update();

    UnixSeconds unixNow() const;
    SkyMath::HorizontalCoord currentHorizontal() const { return _lastHorizontal; }
    StepCount altTargetSteps() const { return _altTargetSteps; }
    StepCount azTargetSteps() const { return _azTargetSteps; }

private:
    void refreshTargetSteps();
    StepCount nearestWrappedAzTarget(StepCount desiredAzSteps) const;

private:
    Clock& _clock;
    AxisController& _altAxis;
    AxisController& _azAxis;
    const MountModel& _altModel;
    const MountModel& _azModel;

    AngleDeg _latDeg = 0.0f;
    AngleDeg _lonDeg = 0.0f;

    UnixSeconds _epochUnix = 0;
    TickMicros _epochMicros = 0;
    TickMicros _lastRefreshMicros = 0;

    SkyMath::EquatorialCoord _target = {0.0f, 0.0f};
    SkyMath::HorizontalCoord _lastHorizontal = {0.0f, 0.0f};
    bool _hasTarget = false;
    bool _tracking = false;

    StepCount _altTargetSteps = 0;
    StepCount _azTargetSteps = 0;
    RateStepsPerSec _gotoRateStepsPerSec = 400.0f;
    TickMicros _refreshIntervalMicros = 1000000UL;
};
