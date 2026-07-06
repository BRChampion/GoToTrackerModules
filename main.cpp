#include <iostream>
#include "core/SimClock.h"
#include "core/FakeDriver.h"
#include "core/AxisController.h"
#include "core/SkyMath.h"
#include "core/TargetCatalog.h"

int main() {
    SimClock clk;
    FakeDriver drv;
    AxisController axis(clk, drv);

    axis.begin();
    axis.enable(true);

    // Test 1: Rate mode at +100 steps/sec for 2 seconds
    axis.startRate(100.0f);

    for (int i = 0; i < 2000; i++) {
        clk.advanceMicros(1000); // 1 ms tick
        axis.update();
    }

    std::cout << "[Rate] posSteps=" << axis.posSteps()
              << " pulses=" << drv.totalPulses << "\n";

    // Test 2: Goto mode to step 1000 at 500 steps/sec
    axis.startGoto(-500, 500.0f);

    for (int i = 0; i < 5000; i++) {
        clk.advanceMicros(1000);
        axis.update();
        if (axis.mode() == AxisController::Mode::Idle) break;
    }

    std::cout << "[Goto] posSteps=" << axis.posSteps()
              << " target=" << axis.targetSteps()
              << " pulses=" << drv.totalPulses << "\n";

    const TargetEq& vega = TargetCatalog::get(0);
    const SkyMath::EquatorialCoord eq = {vega.raHours, vega.decDeg};
    const float lst = SkyMath::lstDegFromUnix(1783353600UL, -52.7f);
    const SkyMath::HorizontalCoord hz = SkyMath::equatorialToHorizontal(eq, lst, 47.6f);

    std::cout << "[AltAz] altDeg=" << hz.altDeg
              << " azDeg=" << hz.azDeg << "\n";
}
