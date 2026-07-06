#include "TargetCatalog.h"

// For future reference:
// If I hard code any more of these on my laptop,
// use "win" + "." to open up the menu, so I can type the "°" symbol

// J2000 RA/Dec values
static const TargetEq TARGETS[] = {
    // Vega: RA 18h 36m 56.336s, Dec +38° 45' 0.1.2802"
    {"Vega",18.61564899, 38.783689},

    // Polaris: RA 02h 31m 49.09s, Dec +89° 15' 50.8"
    {"Polaris", 2.53030278, 89.264111},

    // Betelgeuse: RA 05h 55m 10.30536s, Dec +07° 24' 25.4304"
    {"Betelgeuse", 5.91952927, 7.407064}
};

namespace TargetCatalog {

    const TargetEq* list() {return TARGETS;}

    uint8_t count() {return sizeof(TARGETS)/sizeof(TARGETS[0]);}

    const TargetEq& get(uint8_t i) {return TARGETS[i];}
}
