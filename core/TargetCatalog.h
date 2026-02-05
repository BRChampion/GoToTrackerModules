#pragma once
#include <cstddef>

struct TargetEq {
    const char* name;
    double raHours; // Right Ascension in hours
    double decDeg;  // Declination in degrees
};

namespace TargetCatalog {
    const TargetEq* list();
    std::size_t count();
    const TargetEq& get(std::size_t i);
}


