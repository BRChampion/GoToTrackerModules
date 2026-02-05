#pragma once
#include <cstdint>

class MountModel {
public:
    MountModel(double motorStepsPerRev, double totalRatioMotorToAxis);

    double stepsPerAxisRev() const;
    double stepsPerDeg() const;

    // degrees -> signed steps (rounding to nearest)
    int64_t degToSteps(double deg) const;

private:
    double _motorStepsPerRev;
    double _ratio;
};
