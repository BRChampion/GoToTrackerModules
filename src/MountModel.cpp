#include "MountModel.h"
#include <math.h>

// Axis blueprint for converting mount angles (in deg)
// into motor step counts based on gearing and motor resolution

MountModel::MountModel(float motorStepsPerRev, float totalRatioMotorToAxis)
: _motorStepsPerRev(motorStepsPerRev), _ratio(totalRatioMotorToAxis) {}

// Return # of steps for one entire axis rotation (360 degrees)
float MountModel::stepsPerAxisRev() const {
    return _motorStepsPerRev * _ratio;
}

StepCount MountModel::stepsPerAxisRevRounded() const {
    return (StepCount)lroundf(stepsPerAxisRev());
}

// Return # of steps that correspond to *1* degree of axis rotation
// Used for - converting target sky angles -> step targets
//          - computing tracking rates in steps/sec
float MountModel::stepsPerDeg() const {
    return stepsPerAxisRev() / 360.0f;
}

// Convert an angle (in deg) into the nearest integer motor step position
// NOTE: +/- sign convention is handled by the caller (e.g. +deg = eastward RA motion)
StepCount MountModel::degToSteps(AngleDeg deg) const {
    // nearest integer step
    return (StepCount)lroundf(deg * stepsPerDeg());
}
