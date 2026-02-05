#pragma once

namespace SkyMath {
    // Wrap angle into (0, 360)
    double wrapDeg(double deg);

    // Wrap angle into (-180, +180) -> makes GoTo cleaner
    double wrapSignedDeg(double deg);

    // Unix seconds → Julian date
    double julianDateFromUnix(long long unixSeconds);

    // Julian date + longitude → local sidereal time (deg)
    double lstDeg(double jd, double lonDeg);

    // Hour angle = LST − RA
    double haDeg(double lstDeg, double raDeg);
}
