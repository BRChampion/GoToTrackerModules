#include <iomanip>
#include <iostream>

#include "AltAzTracker.h"
#include "AxisController.h"
#include "FakeDriver.h"
#include "MountModel.h"
#include "SimClock.h"
#include "SkyMath.h"

namespace {

struct DemoCase {
    const char* name;
    const char* command;
    SkyMath::EquatorialCoord target;
    AngleDeg startAzDeg;
};

AngleDeg stepsToAzDeg(const MountModel& model, StepCount steps) {
    return SkyMath::wrapDeg((float)steps / model.stepsPerDeg());
}

void runCase(const DemoCase& item) {
    const uint32_t tickUs = 1000U;
    const uint32_t maxDurationUs = 12000000U;
    const uint32_t j2000Unix = 946728000UL;
    const AngleDeg longitudeForLstZero = -280.46061837f;

    SimClock clock;
    FakeDriver altDriver;
    FakeDriver azDriver;
    AxisController altAxis(clock, altDriver);
    AxisController azAxis(clock, azDriver);
    const MountModel model(200.0f, 16.0f);
    AltAzTracker tracker(clock, altAxis, azAxis, model, model);

    altAxis.begin();
    azAxis.begin();
    altAxis.enable(true);
    azAxis.enable(true);

    tracker.begin();
    tracker.setObserver(0.0f, longitudeForLstZero);
    tracker.setTime(j2000Unix);
    tracker.manualHome();
    azAxis.setPosSteps(model.degToSteps(item.startAzDeg));
    tracker.setTarget(item.target);

    const StepCount startAzSteps = azAxis.posSteps();
    const AngleDeg startAzDeg = stepsToAzDeg(model, startAzSteps);
    const AngleDeg targetAzDeg = tracker.currentHorizontal().azDeg;
    const StepCount targetAzSteps = tracker.azTargetSteps();
    const StepCount deltaSteps = targetAzSteps - startAzSteps;
    const AngleDeg deltaDeg = (float)deltaSteps / model.stepsPerDeg();
    const AngleDeg avoidedDeltaDeg =
        deltaDeg >= 0.0f ? deltaDeg - 360.0f : deltaDeg + 360.0f;

    tracker.startGoto();

    uint32_t durationUs = 0U;
    while (azAxis.mode() != AxisController::Mode::Idle &&
           durationUs < maxDurationUs) {
        clock.advanceMicros(tickUs);
        durationUs += tickUs;
        tracker.update();
    }

    const AngleDeg finalAzDeg = stepsToAzDeg(model, azAxis.posSteps());

    std::cout << "Case: " << item.name << '\n';
    std::cout << "  command: " << item.command << '\n';
    std::cout << "  current azimuth: " << std::fixed << std::setprecision(2)
              << startAzDeg << " deg (" << startAzSteps << " steps)\n";
    std::cout << "  target azimuth:  " << targetAzDeg << " deg ("
              << targetAzSteps << " steps)\n";
    std::cout << "  chosen movement: " << deltaDeg << " deg ("
              << deltaSteps << " steps)\n";
    std::cout << "  avoided long way: " << avoidedDeltaDeg << " deg\n";
    std::cout << "  final azimuth:   " << finalAzDeg << " deg ("
              << azAxis.posSteps() << " steps)\n";
    std::cout << "  pulses issued:   " << azDriver.totalPulses
              << " in " << durationUs << " us\n\n";
}

} // namespace

int main() {
    std::cout << "GoToTracker SIL shortest-path azimuth demo\n";
    std::cout << "Mount model: 200 step/rev motor, 16:1 ratio = 3200 axis steps/rev\n";
    std::cout << "Observer/time chosen so LST = 0 deg and targets land exactly east/west.\n\n";

    const DemoCase cases[] = {
        {
            "wrap north: current 350 deg -> target 90 deg",
            "home; set current az=350; target RA=6h Dec=0; goto",
            {6.0f, 0.0f},
            350.0f
        },
        {
            "wrap north: current 10 deg -> target 270 deg",
            "home; set current az=10; target RA=18h Dec=0; goto",
            {18.0f, 0.0f},
            10.0f
        }
    };

    for (uint8_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        runCase(cases[i]);
    }
    return 0;
}
