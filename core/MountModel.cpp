#include "MountModel.h"
#include <cmath>

// Axis blueprint for converting mount angles (in deg)
// into motor step counts based on gearing and motor resolution

MountModel::MountModel(double motorStepsPerRev, double totalRatioMotorToAxis)
: _motorStepsPerRev(motorStepsPerRev), _ratio(totalRatioMotorToAxis) {}

// Return # of steps for one entire axis rotation (360 degrees)
double MountModel::stepsPerAxisRev() const {
    return _motorStepsPerRev * _ratio;
}

// Return # of steps that correspond to *1* degree of axis rotation
// Used for - converting target sky angles -> step targets
//          - computing tracking rates in steps/sec
double MountModel::stepsPerDeg() const {
    return stepsPerAxisRev() / 360.0;
}

// Convert an angle (in deg) into the nearest integer motor step position
// NOTE: +/- sign convention is handled by the caller (e.g. +deg = eastward RA motion)
int64_t MountModel::degToSteps(double deg) const {
    // nearest integer step
    return (int64_t) llround(deg * stepsPerDeg());
}
