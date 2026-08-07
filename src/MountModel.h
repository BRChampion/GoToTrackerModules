#pragma once
#include "TrackerTypes.h"

class MountModel {
public:
    MountModel(float motorStepsPerRev, float totalRatioMotorToAxis);

    // totalRatioMotorToAxis includes gearbox, belt, worm, and microstepping if
    // the driver is configured to make each input pulse a microstep.
    float stepsPerAxisRev() const;
    StepCount stepsPerAxisRevRounded() const;
    float stepsPerDeg() const;

    // degrees -> signed steps (rounding to nearest)
    StepCount degToSteps(AngleDeg deg) const;

private:
    float _motorStepsPerRev;
    float _ratio;
};
