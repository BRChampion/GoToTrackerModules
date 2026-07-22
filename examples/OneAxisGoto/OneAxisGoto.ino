#include <GoToTracker.h>

constexpr uint8_t StepPin = 2;
constexpr uint8_t DirectionPin = 3;
constexpr uint8_t EnablePin = 4;

ArduinoClock trackerClock;
StepDirDriver motorDriver(StepPin, DirectionPin, EnablePin);
AxisController axis(trackerClock, motorDriver);

// MountModel is independent of the axis controller. Create one per axis when
// altitude and azimuth have different motors or gear ratios.
MountModel mountModel(200.0f, 16.0f);

void setup() {
    Serial.begin(115200);

    motorDriver.begin();
    axis.begin();
    axis.enable(true);

    const StepCount target = mountModel.degToSteps(45.0f);
    axis.startGoto(target, 400.0f);
}

void loop() {
    axis.update();

    static bool reported = false;
    if (!reported && axis.mode() == AxisController::Mode::Idle) {
        Serial.print("Move complete at step ");
        Serial.println(axis.posSteps());
        reported = true;
    }
}
