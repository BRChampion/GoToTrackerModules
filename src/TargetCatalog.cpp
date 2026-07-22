#include "TargetCatalog.h"

// J2000 RA/Dec values.
static const TargetEq TARGETS[] = {
    {"Vega", 18.61564899f, 38.783689f},
    {"Polaris", 2.53030278f, 89.264111f},
    {"Betelgeuse", 5.91952927f, 7.407064f}
};

namespace TargetCatalog {
    const TargetEq* list() { return TARGETS; }

    uint8_t count() {
        return (uint8_t)(sizeof(TARGETS) / sizeof(TARGETS[0]));
    }

    const TargetEq& get(uint8_t i) { return TARGETS[i]; }
}
