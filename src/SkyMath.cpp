#include "SkyMath.h"
#include <math.h>

namespace SkyMath {
    static const float DegToRad = 0.017453292519943295f;
    static const float RadToDeg = 57.29577951308232f;

    // Wrap to [0, 360)
    AngleDeg wrapDeg(AngleDeg deg) {
        deg = fmodf(deg, 360.0f);
        if (deg < 0.0f) deg += 360.0f;
        return deg;
    }

    // Wrap to (-180, +180)
    AngleDeg wrapSignedDeg(AngleDeg deg) {
        deg = wrapDeg(deg);
        if (deg > 180.0f) deg -= 360.0f;
        return deg;
    }

    // Convert Unix Time to Julian Date.
    // Kept for desktop simulation and rough reference only. On AVR, prefer lstDegFromUnix().
    float julianDateFromUnix(UnixSeconds unixSeconds) {
        return unixSeconds / 86400.0f + 2440587.5f;
    }

    // Compute LST in degrees (adapted from astronomical references).
    // Result is wrapped to [0, 360).
    float lstDeg(float jd, AngleDeg lonDeg) {
        float T = (jd - 2451545.0f) / 36525.0f;

        float gst =
            280.46061837f +
            360.98564736629f * (jd - 2451545.0f) +
            0.000387933f * T * T -
            T * T * T / 38710000.0f;

        return wrapDeg(gst + lonDeg);
    }

    AngleDeg lstDegFromUnix(UnixSeconds unixSeconds, AngleDeg lonDeg) {
        const uint32_t secondsPerDay = 86400UL;
        const uint32_t j2000Unix = 946728000UL;

        // Keep the large Unix timestamp in integer math, then split it into
        // smaller day/second values before converting to float. This avoids
        // losing the fractional-day precision that a float Julian Date loses.
        int32_t days;
        uint32_t secondsToday;
        if (unixSeconds >= j2000Unix) {
            const uint32_t elapsed = unixSeconds - j2000Unix;
            days = (int32_t)(elapsed / secondsPerDay);
            secondsToday = elapsed % secondsPerDay;
        } else {
            const uint32_t elapsed = j2000Unix - unixSeconds;
            const uint32_t remainder = elapsed % secondsPerDay;
            days = -(int32_t)(elapsed / secondsPerDay);
            if (remainder != 0U) {
                --days;
                secondsToday = secondsPerDay - remainder;
            } else {
                secondsToday = 0U;
            }
        }

        float gst =
            280.46061837f +
            0.98564736629f * (float)days +
            0.004178074622f * (float)secondsToday;

        return wrapDeg(gst + lonDeg);
    }

    // Compute HA in degrees, return in signed form.
    // Sign determines direction of rotation.
    AngleDeg haDeg(AngleDeg lstDegVal, AngleDeg raDegVal) {
        return wrapSignedDeg(lstDegVal - raDegVal);
    }

    AngleDeg raHoursToDeg(float raHours) {
        return wrapDeg(raHours * 15.0f);
    }

    HorizontalCoord equatorialToHorizontal(
        EquatorialCoord target,
        AngleDeg lstDegVal,
        AngleDeg observerLatDeg) {

        const float haRad = haDeg(lstDegVal, raHoursToDeg(target.raHours)) * DegToRad;
        const float decRad = target.decDeg * DegToRad;
        const float latRad = observerLatDeg * DegToRad;

        const float sinDec = sinf(decRad);
        const float cosDec = cosf(decRad);
        const float sinLat = sinf(latRad);
        const float cosLat = cosf(latRad);
        const float sinHa = sinf(haRad);
        const float cosHa = cosf(haRad);

        float sinAlt = sinDec * sinLat + cosDec * cosLat * cosHa;
        // Clamp small float overshoots so asinf() stays in its valid domain.
        if (sinAlt > 1.0f) sinAlt = 1.0f;
        if (sinAlt < -1.0f) sinAlt = -1.0f;

        HorizontalCoord out;
        out.altDeg = asinf(sinAlt) * RadToDeg;
        out.azDeg = wrapDeg(atan2f(-cosDec * sinHa,
                                   sinDec * cosLat - cosDec * sinLat * cosHa) * RadToDeg);
        return out;
    }
}
