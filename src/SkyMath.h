#pragma once
#include "TrackerTypes.h"

namespace SkyMath {
    // Catalog and command inputs are equatorial; Alt/Az mounts need horizontal
    // coordinates computed for the observer's time and location.
    struct EquatorialCoord {
        float raHours;
        AngleDeg decDeg;
    };

    struct HorizontalCoord {
        AngleDeg altDeg;
        AngleDeg azDeg;
    };

    // Wrap angle into (0, 360)
    AngleDeg wrapDeg(AngleDeg deg);

    // Wrap angle into (-180, +180) -> makes GoTo cleaner
    AngleDeg wrapSignedDeg(AngleDeg deg);

    // Unix seconds -> Julian date. Prefer lstDegFromUnix() on AVR.
    float julianDateFromUnix(UnixSeconds unixSeconds);

    // Julian date + longitude -> local sidereal time (deg)
    float lstDeg(float jd, AngleDeg lonDeg);

    // Unix seconds + longitude -> local sidereal time (deg), AVR-friendlier.
    AngleDeg lstDegFromUnix(UnixSeconds unixSeconds, AngleDeg lonDeg);

    // Hour angle = LST - RA
    AngleDeg haDeg(AngleDeg lstDeg, AngleDeg raDeg);

    AngleDeg raHoursToDeg(float raHours);

    // Convert RA/Dec target to Alt/Az for a local observer.
    HorizontalCoord equatorialToHorizontal(
        EquatorialCoord target,
        AngleDeg lstDeg,
        AngleDeg observerLatDeg);
}
