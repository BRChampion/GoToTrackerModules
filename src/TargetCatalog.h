#pragma once
#include <stdint.h>

struct TargetEq {
    const char* name;
    float raHours; // Right Ascension in hours
    float decDeg;  // Declination in degrees
};

namespace TargetCatalog {
    const TargetEq* list();
    uint8_t count();
    const TargetEq& get(uint8_t i);
}


