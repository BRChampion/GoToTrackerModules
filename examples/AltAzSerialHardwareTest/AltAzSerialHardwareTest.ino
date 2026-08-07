#include <GoToTracker.h>

// One-motor Alt/Az hardware test for Arduino MEGA + TB6600 + NEMA 17.
// Set TestAxis to AxisAz or AxisAlt, upload, mark the motor shaft, run
// "home", choose a target, then compare the physical movement to "status".

enum TestAxis {
    AxisAlt,
    AxisAz
};

// ----- Hardware settings -----
constexpr TestAxis TestAxisMode = AxisAz;

constexpr uint8_t StepPin = 2;
constexpr uint8_t DirectionPin = 3;
constexpr uint8_t EnablePin = 4;

// TB6600 modules are often enable-active-low, but not always. If the motor is
// disabled when it should be enabled, change this to false.
constexpr bool EnableActiveLow = true;
constexpr uint16_t StepPulseMicros = 5;

// For a bare NEMA 17 shaft: 200 full steps/rev. If TB6600 is set to 1/8
// microstepping, use 1600. If geared, put the remaining axis ratio below.
constexpr float MotorStepsPerRev = 200.0f;
constexpr float AltMotorToAxisRatio = 1.0f;
constexpr float AzMotorToAxisRatio = 1.0f;

// ----- Observer/time settings -----
// Update these before upload. Longitude is negative west.
constexpr AngleDeg ObserverLatDeg = 47.6f;
constexpr AngleDeg ObserverLonDeg = -52.7f;

// Fixed UTC Unix time for repeatable bench tests. Replace with your test time.
// Example value: 2026-07-06 12:00:00 UTC.
constexpr UnixSeconds StartupUnixTime = 1783339200UL;

ArduinoClock trackerClock;
StepDirDriver physicalDriver(
    StepPin,
    DirectionPin,
    EnablePin,
    EnableActiveLow,
    StepPulseMicros);

class NullDriver : public Driver {
public:
    void enable(bool) override {}
    void step(StepDir) override {}
};

NullDriver nullDriver;

AxisController altAxis(
    trackerClock,
    TestAxisMode == AxisAlt ? (Driver&)physicalDriver : (Driver&)nullDriver);
AxisController azAxis(
    trackerClock,
    TestAxisMode == AxisAz ? (Driver&)physicalDriver : (Driver&)nullDriver);

MountModel altModel(MotorStepsPerRev, AltMotorToAxisRatio);
MountModel azModel(MotorStepsPerRev, AzMotorToAxisRatio);
AltAzTracker tracker(trackerClock, altAxis, azAxis, altModel, azModel);

StreamCommandOutput commandOutput(Serial);
CommandInterface commands(tracker, altAxis, azAxis, commandOutput);
StreamCommandInput commandInput(Serial, commands);

void printStartup() {
    Serial.println();
    Serial.println("GoToTracker Alt/Az serial hardware test");
    Serial.print("Physical motor axis: ");
    Serial.println(TestAxisMode == AxisAz ? "azimuth" : "altitude");
    Serial.println("Commands: help, status, home, target <index|name>, goto, stop");
    Serial.println("Suggested flow: home -> target Vega -> goto -> status");
    Serial.println();
}

void setup() {
    Serial.begin(115200);

    physicalDriver.begin();
    altAxis.begin();
    azAxis.begin();
    altAxis.enable(true);
    azAxis.enable(true);

    tracker.begin();
    tracker.setObserver(ObserverLatDeg, ObserverLonDeg);
    tracker.setTime(StartupUnixTime);

    printStartup();
}

void loop() {
    commandInput.update();
    tracker.update();
}
