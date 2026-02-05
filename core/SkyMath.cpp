#include "SkyMath.h"
#include <cmath>

namespace SkyMath {

    // Wrap to (0, 360)
    double wrapDeg(double deg) {
        while (deg < 0) deg += 360.0;
        while (deg >= 360.0) deg -= 360.0;
        return deg;
    }

    // Wrap to (-180, +180)
    double wrapSignedDeg(double deg) {
        deg = wrapDeg(deg);
        if (deg > 180.0) deg -= 360.0;
        return deg;
    }

    // Convert Unix Time to Julian Date
    double julianDateFromUnix(long long unixSeconds) {
        return unixSeconds / 86400.0 + 2440587.5;
    }

    // Compute LST in degrees (adapted from astronomical references)
    // Result is wrapped to (0, 360)
    double lstDeg(double jd, double lonDeg) {
        double T = (jd - 2451545.0) / 36525.0;

        double gst =
            280.46061837 +
            360.98564736629 * (jd - 2451545.0) +
            0.000387933 * T * T -
            T * T * T / 38710000.0;

        return wrapDeg(gst + lonDeg);
    }

    // Compute HA in degrees, return in signed form
    // Sign determines direction of rotation
    double haDeg(double lstDegVal, double raDegVal) {
        return wrapSignedDeg(lstDegVal - raDegVal);
    }

}
