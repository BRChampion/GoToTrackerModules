#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>

#include "AltAzTracker.h"
#include "AxisController.h"
#include "FakeDriver.h"
#include "MountModel.h"
#include "SimClock.h"
#include "SkyMath.h"
#include "TargetCatalog.h"

namespace {

struct TestRun {
    std::ofstream trace;
    std::ofstream summary;
    std::ofstream comparisons;
    unsigned failures = 0;

    TestRun(const char* tracePath,
            const char* summaryPath,
            const char* comparisonPath)
        : trace(tracePath),
          summary(summaryPath),
          comparisons(comparisonPath) {
        trace << "scenario,step_index,elapsed_us,clock_micros,pos_steps,"
                 "driver_pos_steps,total_pulses\n";
        summary << "scenario,command,command_value,tick_us,"
                   "simulated_duration_us,start_pos,final_pos,target_pos,"
                   "scenario_pulses,cumulative_pulses,first_step_us,"
                   "last_step_us,passed\n";
        comparisons << "category,case_id,inputs,metric,units,expected,"
                       "calculated,tolerance,absolute_error,passed\n";
        comparisons << std::setprecision(10);
    }

    void check(bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }

    void logStep(const char* scenario,
                 uint32_t stepIndex,
                 uint32_t elapsedUs,
                 const SimClock& clock,
                 const AxisController& axis,
                 const FakeDriver& driver) {
        trace << scenario << ',' << stepIndex << ',' << elapsedUs << ','
              << clock.micros() << ',' << axis.posSteps() << ','
              << driver.posSteps << ',' << driver.totalPulses << '\n';
    }

    void logSummary(const char* scenario,
                    const char* command,
                    float commandValue,
                    uint32_t tickUs,
                    uint32_t durationUs,
                    StepCount startPos,
                    StepCount finalPos,
                    StepCount targetPos,
                    uint32_t scenarioPulses,
                    uint32_t cumulativePulses,
                    uint32_t firstStepUs,
                    uint32_t lastStepUs,
                    bool passed) {
        summary << scenario << ',' << command << ',' << commandValue << ','
                << tickUs << ',' << durationUs << ',' << startPos << ','
                << finalPos << ',' << targetPos << ',' << scenarioPulses << ','
                << cumulativePulses << ',' << firstStepUs << ',' << lastStepUs
                << ',' << (passed ? "true" : "false") << '\n';
    }

    bool compare(const char* category,
                 const char* caseId,
                 const char* inputs,
                 const char* metric,
                 const char* units,
                 double expected,
                 double calculated,
                 double tolerance,
                 bool circularDegrees = false) {
        double error = fabs(calculated - expected);
        if (circularDegrees && error > 180.0) error = 360.0 - error;
        const bool passed = error <= tolerance;
        if (!passed) {
            ++failures;
            std::cerr << "FAIL: " << caseId << ' ' << metric
                      << " expected=" << expected
                      << " calculated=" << calculated
                      << " tolerance=" << tolerance << '\n';
        }

        comparisons << category << ',' << caseId << ',' << inputs << ','
                    << metric << ',' << units << ',' << expected << ','
                    << calculated << ',' << tolerance << ',' << error << ','
                    << (passed ? "true" : "false") << '\n';
        return passed;
    }
};

void runOriginalScenarios(TestRun& run) {
    const uint32_t tickUs = 1000U;
    SimClock clock;
    FakeDriver driver;
    AxisController axis(clock, driver);
    axis.begin();
    axis.enable(true);

    const StepCount rateStart = axis.posSteps();
    const uint32_t ratePulseStart = driver.totalPulses;
    uint32_t rateFirstStep = 0U;
    uint32_t rateLastStep = 0U;
    uint32_t rateStepIndex = 0U;
    axis.startRate(100.0f);

    for (uint32_t elapsed = tickUs; elapsed <= 2000000U; elapsed += tickUs) {
        clock.advanceMicros(tickUs);
        if (axis.update()) {
            if (rateFirstStep == 0U) rateFirstStep = elapsed;
            rateLastStep = elapsed;
            run.logStep("original_rate_100", ++rateStepIndex, elapsed,
                        clock, axis, driver);
        }
    }

    const uint32_t ratePulses = driver.totalPulses - ratePulseStart;
    const bool ratePassed = axis.posSteps() == 200 &&
                            driver.posSteps == 200 &&
                            ratePulses == 200U;
    run.check(ratePassed, "original +100 steps/s scenario");
    run.compare("timing_rate", "exact_rate_100", "rate=100;tick_us=1000;duration_us=2000000",
                "pulse_count", "pulses", 200.0, ratePulses, 0.0);
    run.compare("timing_rate", "exact_rate_100", "rate=100;tick_us=1000;duration_us=2000000",
                "final_position", "steps", 200.0, axis.posSteps(), 0.0);
    run.compare("timing_rate", "exact_rate_100", "rate=100;tick_us=1000;duration_us=2000000",
                "first_step_time", "us", 10000.0, rateFirstStep, 0.0);
    run.logSummary("original_rate_100", "rate_steps_per_sec", 100.0f,
                   tickUs, 2000000U, rateStart, axis.posSteps(), 0,
                   ratePulses, driver.totalPulses, rateFirstStep, rateLastStep,
                   ratePassed);

    const StepCount gotoStart = axis.posSteps();
    const uint32_t gotoPulseStart = driver.totalPulses;
    uint32_t gotoFirstStep = 0U;
    uint32_t gotoLastStep = 0U;
    uint32_t gotoStepIndex = 0U;
    uint32_t gotoDuration = 0U;
    axis.startGoto(-500, 500.0f);

    for (uint32_t elapsed = tickUs; elapsed <= 5000000U; elapsed += tickUs) {
        clock.advanceMicros(tickUs);
        gotoDuration = elapsed;
        if (axis.update()) {
            if (gotoFirstStep == 0U) gotoFirstStep = elapsed;
            gotoLastStep = elapsed;
            run.logStep("original_goto_negative_500", ++gotoStepIndex, elapsed,
                        clock, axis, driver);
        }
        if (axis.mode() == AxisController::Mode::Idle) break;
    }

    const uint32_t gotoPulses = driver.totalPulses - gotoPulseStart;
    const bool gotoPassed = axis.posSteps() == -500 &&
                            driver.posSteps == -500 &&
                            axis.targetSteps() == -500 &&
                            gotoPulses == 700U &&
                            driver.totalPulses == 900U &&
                            gotoDuration == 1400000U;
    run.check(gotoPassed, "original goto -500 scenario");
    run.compare("timing_rate", "original_goto_negative_500",
                "start=200;target=-500;rate=500;tick_us=1000",
                "move_duration", "us", 1400000.0, gotoDuration, 0.0);
    run.compare("timing_rate", "original_goto_negative_500",
                "start=200;target=-500;rate=500;tick_us=1000",
                "pulse_count", "pulses", 700.0, gotoPulses, 0.0);
    run.logSummary("original_goto_negative_500", "goto_max_steps_per_sec",
                   500.0f, tickUs, gotoDuration, gotoStart, axis.posSteps(),
                   -500, gotoPulses, driver.totalPulses, gotoFirstStep,
                   gotoLastStep, gotoPassed);
}

void runFractionalNegativeRate(TestRun& run) {
    const uint32_t tickUs = 1000U;
    SimClock clock;
    FakeDriver driver;
    AxisController axis(clock, driver);
    axis.begin();
    axis.enable(true);
    axis.startRate(-37.5f);

    uint32_t firstStep = 0U;
    uint32_t lastStep = 0U;
    uint32_t stepIndex = 0U;
    for (uint32_t elapsed = tickUs; elapsed <= 4000000U; elapsed += tickUs) {
        clock.advanceMicros(tickUs);
        if (axis.update()) {
            if (firstStep == 0U) firstStep = elapsed;
            lastStep = elapsed;
            run.logStep("fractional_rate_negative_37_5", ++stepIndex, elapsed,
                        clock, axis, driver);
        }
    }

    const bool passed = axis.posSteps() == -150 &&
                        driver.posSteps == -150 &&
                        driver.totalPulses == 150U;
    run.check(passed, "fractional negative-rate scenario");
    run.compare("timing_rate", "fractional_rate_negative_37_5",
                "rate=-37.5;tick_us=1000;duration_us=4000000",
                "pulse_count", "pulses", 150.0, driver.totalPulses, 0.0);
    run.compare("timing_rate", "fractional_rate_negative_37_5",
                "rate=-37.5;tick_us=1000;duration_us=4000000",
                "final_position", "steps", -150.0, axis.posSteps(), 0.0);
    run.logSummary("fractional_rate_negative_37_5", "rate_steps_per_sec",
                   -37.5f, tickUs, 4000000U, 0, axis.posSteps(), 0,
                   driver.totalPulses, driver.totalPulses, firstStep, lastStep,
                   passed);
}

void runMicrosRollover(TestRun& run) {
    const uint32_t tickUs = 1000U;
    SimClock clock;
    clock.advanceMicros(std::numeric_limits<uint32_t>::max() - 5000U);

    FakeDriver driver;
    AxisController axis(clock, driver);
    axis.begin();
    axis.enable(true);
    axis.startRate(1000.0f);

    uint32_t firstStep = 0U;
    uint32_t lastStep = 0U;
    uint32_t stepIndex = 0U;
    for (uint32_t elapsed = tickUs; elapsed <= 10000U; elapsed += tickUs) {
        clock.advanceMicros(tickUs);
        if (axis.update()) {
            if (firstStep == 0U) firstStep = elapsed;
            lastStep = elapsed;
            run.logStep("micros_rollover_rate_1000", ++stepIndex, elapsed,
                        clock, axis, driver);
        }
    }

    const bool passed = axis.posSteps() == 10 &&
                        driver.posSteps == 10 &&
                        driver.totalPulses == 10U &&
                        clock.micros() == 4999U;
    run.check(passed, "32-bit micros rollover scenario");
    run.compare("timing_rate", "micros_rollover_rate_1000",
                "rate=1000;tick_us=1000;start_clock=4294962295",
                "pulse_count", "pulses", 10.0, driver.totalPulses, 0.0);
    run.compare("timing_rate", "micros_rollover_rate_1000",
                "rate=1000;tick_us=1000;start_clock=4294962295",
                "wrapped_clock", "us", 4999.0, clock.micros(), 0.0);
    run.logSummary("micros_rollover_rate_1000", "rate_steps_per_sec",
                   1000.0f, tickUs, 10000U, 0, axis.posSteps(), 0,
                   driver.totalPulses, driver.totalPulses, firstStep, lastStep,
                   passed);
}

void runMountModels(TestRun& run) {
    const MountModel firstAxis(200.0f, 16.0f);
    const MountModel secondAxis(200.0f, 32.0f);
    const StepCount firstTarget = firstAxis.degToSteps(45.0f);
    const StepCount secondTarget = secondAxis.degToSteps(45.0f);
    const bool firstPassed = firstTarget == 400;
    const bool secondPassed = secondTarget == 800;

    run.check(firstPassed, "first-axis MountModel conversion");
    run.check(secondPassed, "second-axis MountModel conversion");
    run.compare("angle_steps", "mount_model_ratio_16",
                "motor_steps=200;ratio=16;angle=45",
                "step_position", "steps", 400.0, firstTarget, 0.0);
    run.compare("angle_steps", "mount_model_ratio_32",
                "motor_steps=200;ratio=32;angle=45",
                "step_position", "steps", 800.0, secondTarget, 0.0);
    run.logSummary("mount_model_ratio_16", "angle_degrees", 45.0f, 0U, 0U,
                   0, firstTarget, firstTarget, 0U, 0U, 0U, 0U, firstPassed);
    run.logSummary("mount_model_ratio_32", "angle_degrees", 45.0f, 0U, 0U,
                   0, secondTarget, secondTarget, 0U, 0U, 0U, 0U,
                   secondPassed);
}

void runAstronomicalCoordinates(TestRun& run) {
    struct CoordinateCase {
        const char* id;
        const char* inputs;
        SkyMath::EquatorialCoord target;
        AngleDeg lstDeg;
        AngleDeg latitudeDeg;
        double expectedAltitudeDeg;
        double expectedAzimuthDeg;
    };

    const CoordinateCase cases[] = {
        {"equator_on_south_meridian", "ra_h=0;dec_deg=0;lst_deg=0;lat_deg=45",
         {0.0f, 0.0f}, 0.0f, 45.0f, 45.0, 180.0},
        {"equator_east_horizon", "ra_h=6;dec_deg=0;lst_deg=0;lat_deg=0",
         {6.0f, 0.0f}, 0.0f, 0.0f, 0.0, 90.0},
        {"equator_west_horizon", "ra_h=18;dec_deg=0;lst_deg=0;lat_deg=0",
         {18.0f, 0.0f}, 0.0f, 0.0f, 0.0, 270.0},
        {"general_northwest_case", "ra_h=18;dec_deg=45;lst_deg=0;lat_deg=45",
         {18.0f, 45.0f}, 0.0f, 45.0f, 30.0, 305.2643896828}
    };

    for (uint8_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const CoordinateCase& item = cases[i];
        const SkyMath::HorizontalCoord calculated =
            SkyMath::equatorialToHorizontal(item.target,
                                             item.lstDeg,
                                             item.latitudeDeg);
        run.compare("astronomical_coordinates", item.id, item.inputs,
                    "altitude", "deg", item.expectedAltitudeDeg,
                    calculated.altDeg, 0.001);
        run.compare("astronomical_coordinates", item.id, item.inputs,
                    "azimuth", "deg", item.expectedAzimuthDeg,
                    calculated.azDeg, 0.001, true);
    }

    const AngleDeg lst = SkyMath::lstDegFromUnix(1783353600UL, -52.7f);
    run.compare("astronomical_coordinates", "newfoundland_lst_reference",
                "unix=1783353600;longitude_deg=-52.7",
                "local_sidereal_time", "deg", 111.9483679258, lst, 0.01,
                true);
}

void runAngleWrapping(TestRun& run) {
    struct WrapCase {
        const char* id;
        float input;
        float expected;
    };

    const WrapCase unsignedCases[] = {
        {"wrap_negative_one", -1.0f, 359.0f},
        {"wrap_positive_360", 360.0f, 0.0f},
        {"wrap_positive_721", 721.0f, 1.0f},
        {"wrap_negative_721", -721.0f, 359.0f}
    };
    for (uint8_t i = 0U; i < sizeof(unsignedCases) / sizeof(unsignedCases[0]); ++i) {
        const WrapCase& item = unsignedCases[i];
        run.compare("angle_steps", item.id, "function=wrapDeg",
                    "wrapped_angle", "deg", item.expected,
                    SkyMath::wrapDeg(item.input), 0.0001, true);
    }

    const WrapCase signedCases[] = {
        {"signed_wrap_181", 181.0f, -179.0f},
        {"signed_wrap_negative_181", -181.0f, 179.0f},
        {"signed_wrap_540", 540.0f, 180.0f}
    };
    for (uint8_t i = 0U; i < sizeof(signedCases) / sizeof(signedCases[0]); ++i) {
        const WrapCase& item = signedCases[i];
        run.compare("angle_steps", item.id, "function=wrapSignedDeg",
                    "wrapped_angle", "deg", item.expected,
                    SkyMath::wrapSignedDeg(item.input), 0.0001);
    }

    const MountModel model(200.0f, 16.0f);
    run.compare("angle_steps", "wrapped_negative_one_to_steps",
                "angle_deg=-1;wrapped_deg=359;steps_per_rev=3200",
                "step_position", "steps", 3191.0,
                model.degToSteps(SkyMath::wrapDeg(-1.0f)), 0.0);
    run.compare("angle_steps", "wrapped_361_to_steps",
                "angle_deg=361;wrapped_deg=1;steps_per_rev=3200",
                "step_position", "steps", 9.0,
                model.degToSteps(SkyMath::wrapDeg(361.0f)), 0.0);
    run.compare("angle_steps", "wrapped_720_to_steps",
                "angle_deg=720;wrapped_deg=0;steps_per_rev=3200",
                "step_position", "steps", 0.0,
                model.degToSteps(SkyMath::wrapDeg(720.0f)), 0.0);
}

void runShortestAzimuthWrapping(TestRun& run) {
    const uint32_t j2000Unix = 946728000UL;
    const AngleDeg longitudeForLstZero = -280.46061837f;
    const MountModel model(200.0f, 16.0f);

    struct AzimuthCase {
        const char* id;
        const char* inputs;
        float raHours;
        StepCount initialAzSteps;
        StepCount expectedTargetSteps;
    };
    const AzimuthCase cases[] = {
        {"shortest_350_to_90", "current_deg=350;desired_deg=90;steps_per_rev=3200",
         6.0f, model.degToSteps(350.0f), 4000},
        {"shortest_10_to_270", "current_deg=10;desired_deg=270;steps_per_rev=3200",
         18.0f, model.degToSteps(10.0f), -800}
    };

    for (uint8_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        SimClock clock;
        FakeDriver altDriver;
        FakeDriver azDriver;
        AxisController altAxis(clock, altDriver);
        AxisController azAxis(clock, azDriver);
        AltAzTracker tracker(clock, altAxis, azAxis, model, model);
        azAxis.setPosSteps(cases[i].initialAzSteps);
        tracker.begin();
        tracker.setObserver(0.0f, longitudeForLstZero);
        tracker.setTime(j2000Unix);
        const SkyMath::EquatorialCoord target = {cases[i].raHours, 0.0f};
        tracker.setTarget(target);

        run.compare("angle_steps", cases[i].id, cases[i].inputs,
                    "azimuth_target", "steps", cases[i].expectedTargetSteps,
                    tracker.azTargetSteps(), 0.0);
    }
}

void runManualHomeAndOneStarCalibration(TestRun& run) {
    const MountModel model(200.0f, 16.0f);
    SimClock clock;
    FakeDriver altDriver;
    FakeDriver azDriver;
    AxisController altAxis(clock, altDriver);
    AxisController azAxis(clock, azDriver);
    AltAzTracker tracker(clock, altAxis, azAxis, model, model);

    altAxis.begin();
    azAxis.begin();
    altAxis.enable(true);
    azAxis.enable(true);
    altAxis.setPosSteps(1234);
    azAxis.setPosSteps(-987);

    tracker.begin();
    tracker.setObserver(47.6f, -52.7f);
    tracker.setTime(1783353600UL);

    tracker.manualHome();
    run.compare("calibration", "manual_home", "initial_alt=1234;initial_az=-987",
                "altitude_position", "steps", 0.0, altAxis.posSteps(), 0.0);
    run.compare("calibration", "manual_home", "initial_alt=1234;initial_az=-987",
                "azimuth_position", "steps", 0.0, azAxis.posSteps(), 0.0);
    run.compare("calibration", "manual_home", "initial_alt=1234;initial_az=-987",
                "homed_flag", "bool", 1.0, tracker.homed() ? 1.0 : 0.0, 0.0);
    run.compare("calibration", "manual_home", "initial_alt=1234;initial_az=-987",
                "calibrated_flag", "bool", 0.0,
                tracker.calibrated() ? 1.0 : 0.0, 0.0);

    const bool syncWithoutTarget = tracker.syncOneStar();
    run.compare("calibration", "one_star_without_target", "target=none",
                "sync_result", "bool", 0.0, syncWithoutTarget ? 1.0 : 0.0,
                0.0);

    const TargetEq& vega = TargetCatalog::get(0U);
    const SkyMath::EquatorialCoord target = {vega.raHours, vega.decDeg};
    tracker.setTarget(target);

    altAxis.setPosSteps(41);
    azAxis.setPosSteps(120);
    const bool syncWithTarget = tracker.syncOneStar();

    run.compare("calibration", "one_star_sync_vega",
                "target=Vega;unix=1783353600;lat_deg=47.6;lon_deg=-52.7",
                "sync_result", "bool", 1.0, syncWithTarget ? 1.0 : 0.0,
                0.0);
    run.compare("calibration", "one_star_sync_vega",
                "target=Vega;unix=1783353600;lat_deg=47.6;lon_deg=-52.7",
                "altitude_position", "steps", -26.0, altAxis.posSteps(),
                0.0);
    run.compare("calibration", "one_star_sync_vega",
                "target=Vega;unix=1783353600;lat_deg=47.6;lon_deg=-52.7",
                "azimuth_position", "steps", 88.0, azAxis.posSteps(), 0.0);
    run.compare("calibration", "one_star_sync_vega",
                "target=Vega;unix=1783353600;lat_deg=47.6;lon_deg=-52.7",
                "calibrated_flag", "bool", 1.0,
                tracker.calibrated() ? 1.0 : 0.0, 0.0);
}

void runAdditionalTimingCases(TestRun& run) {
    struct TimingCase {
        const char* id;
        const char* inputs;
        float rate;
        uint32_t tickUs;
        uint32_t durationUs;
        int32_t expectedPosition;
        uint32_t expectedPulses;
        uint32_t expectedFirstStepUs;
    };
    const TimingCase cases[] = {
        {"quantized_rate_333_3", "rate=333.3;tick_us=100;duration_us=3000000",
         333.3f, 100U, 3000000U, 1000, 1000U, 3000U},
        {"undersampled_rate_1000", "rate=1000;tick_us=5000;duration_us=100000",
         1000.0f, 5000U, 100000U, 20, 20U, 5000U}
    };

    for (uint8_t i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const TimingCase& item = cases[i];
        SimClock clock;
        FakeDriver driver;
        AxisController axis(clock, driver);
        axis.begin();
        axis.enable(true);
        axis.startRate(item.rate);
        uint32_t firstStepUs = 0U;

        for (uint32_t elapsed = item.tickUs;
             elapsed <= item.durationUs;
             elapsed += item.tickUs) {
            clock.advanceMicros(item.tickUs);
            if (axis.update() && firstStepUs == 0U) firstStepUs = elapsed;
        }

        run.compare("timing_rate", item.id, item.inputs,
                    "pulse_count", "pulses", item.expectedPulses,
                    driver.totalPulses, 0.0);
        run.compare("timing_rate", item.id, item.inputs,
                    "final_position", "steps", item.expectedPosition,
                    axis.posSteps(), 0.0);
        run.compare("timing_rate", item.id, item.inputs,
                    "first_step_time", "us", item.expectedFirstStepUs,
                    firstStepUs, 0.0);
    }
}

void runCatalogEndToEndCase(TestRun& run,
                            uint8_t catalogIndex,
                            const char* caseId,
                            const char* inputs,
                            double expectedAltitudeDeg,
                            double expectedAzimuthDeg,
                            StepCount expectedAltSteps,
                            StepCount expectedAzSteps,
                            uint32_t expectedDurationUs) {
    const uint32_t tickUs = 500U;
    SimClock clock;
    FakeDriver altDriver;
    FakeDriver azDriver;
    AxisController altAxis(clock, altDriver);
    AxisController azAxis(clock, azDriver);
    const MountModel altModel(200.0f, 16.0f);
    const MountModel azModel(200.0f, 16.0f);
    AltAzTracker tracker(clock, altAxis, azAxis, altModel, azModel);

    altAxis.begin();
    azAxis.begin();
    altAxis.enable(true);
    azAxis.enable(true);
    tracker.begin();
    tracker.setObserver(47.6f, -52.7f);
    tracker.setTime(1783353600UL);

    const TargetEq& catalogTarget = TargetCatalog::get(catalogIndex);
    const SkyMath::EquatorialCoord target = {
        catalogTarget.raHours, catalogTarget.decDeg
    };
    tracker.setTarget(target);
    const SkyMath::HorizontalCoord horizontal = tracker.currentHorizontal();
    tracker.startGoto();

    uint32_t durationUs = 0U;
    while ((altAxis.mode() != AxisController::Mode::Idle ||
            azAxis.mode() != AxisController::Mode::Idle) &&
           durationUs < 5000000U) {
        clock.advanceMicros(tickUs);
        durationUs += tickUs;
        tracker.update();
    }

    run.compare("end_to_end", caseId, inputs, "altitude", "deg",
                expectedAltitudeDeg, horizontal.altDeg, 0.01);
    run.compare("end_to_end", caseId, inputs, "azimuth", "deg",
                expectedAzimuthDeg, horizontal.azDeg, 0.01, true);
    run.compare("end_to_end", caseId, inputs, "altitude_target", "steps",
                expectedAltSteps, tracker.altTargetSteps(), 0.0);
    run.compare("end_to_end", caseId, inputs, "azimuth_target", "steps",
                expectedAzSteps, tracker.azTargetSteps(), 0.0);
    run.compare("end_to_end", caseId, inputs, "altitude_final", "steps",
                expectedAltSteps, altAxis.posSteps(), 0.0);
    run.compare("end_to_end", caseId, inputs, "azimuth_final", "steps",
                expectedAzSteps, azAxis.posSteps(), 0.0);
    run.compare("end_to_end", caseId, inputs, "altitude_pulses", "pulses",
                fabs((double)expectedAltSteps), altDriver.totalPulses, 0.0);
    run.compare("end_to_end", caseId, inputs, "azimuth_pulses", "pulses",
                fabs((double)expectedAzSteps), azDriver.totalPulses, 0.0);
    run.compare("end_to_end", caseId, inputs, "completion_time", "us",
                expectedDurationUs, durationUs, 0.0);
}

void runCatalogEndToEnd(TestRun& run) {
    const char* vegaInputs = "target=Vega;ra_h=18.61564899;dec_deg=38.783689;unix=1783353600;lat_deg=47.6;lon_deg=-52.7;motor_steps=200;ratio=16;rate=400;tick_us=500";
    const char* polarisInputs = "target=Polaris;ra_h=2.53030278;dec_deg=89.264111;unix=1783353600;lat_deg=47.6;lon_deg=-52.7;motor_steps=200;ratio=16;rate=400;tick_us=500";
    runCatalogEndToEndCase(run, 0U, "catalog_vega_to_motors", vegaInputs,
                           -2.8767321101, 9.8908106895, -26, 88, 220000U);
    runCatalogEndToEndCase(run, 1U, "catalog_polaris_to_motors", polarisInputs,
                           47.7981093169, 358.9469510188, 425, -9, 1062500U);
}

} // namespace

int main(int argc, char** argv) {
    const char* tracePath = argc > 1 ? argv[1] : "desktop_sil_trace.csv";
    const char* summaryPath = argc > 2 ? argv[2] : "desktop_sil_summary.csv";
    const char* comparisonPath =
        argc > 3 ? argv[3] : "desktop_sil_comparisons.csv";
    TestRun run(tracePath, summaryPath, comparisonPath);

    if (!run.trace || !run.summary || !run.comparisons) {
        std::cerr << "Unable to open SIL output files\n";
        return 2;
    }

    runOriginalScenarios(run);
    runFractionalNegativeRate(run);
    runMicrosRollover(run);
    runMountModels(run);
    runAstronomicalCoordinates(run);
    runAngleWrapping(run);
    runShortestAzimuthWrapping(run);
    runManualHomeAndOneStarCalibration(run);
    runAdditionalTimingCases(run);
    runCatalogEndToEnd(run);

    if (run.failures != 0U) {
        std::cerr << run.failures << " SIL test(s) failed\n";
        return 1;
    }

    std::cout << "All SIL tests passed\n"
              << "Trace: " << tracePath << '\n'
              << "Summary: " << summaryPath << '\n'
              << "Comparisons: " << comparisonPath << '\n';
    return 0;
}
