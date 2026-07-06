#pragma once
#include "TrackerTypes.h"

class MountModel {
public:
    MountModel(float motorStepsPerRev, float totalRatioMotorToAxis);

    float stepsPerAxisRev() const;
    float stepsPerDeg() const;

    // degrees -> signed steps (rounding to nearest)
    StepCount degToSteps(AngleDeg deg) const;

private:
    float _motorStepsPerRev;
    float _ratio;
};
