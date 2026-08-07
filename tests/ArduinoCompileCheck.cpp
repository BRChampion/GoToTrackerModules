#include <GoToTracker.h>

void compileArduinoSurface(Stream& serial) {
    ArduinoClock clock;
    StepDirDriver driver(2U, 3U, 4U);
    StepDirDriver azDriver(5U, 6U, 7U, true, 5U);
    AxisController axis(clock, driver);
    AxisController azAxis(clock, azDriver);
    MountModel model(200.0f, 16.0f);
    AltAzTracker tracker(clock, axis, azAxis, model, model);
    StreamCommandOutput output(serial);
    CommandInterface commands(tracker, axis, azAxis, output);
    StreamCommandInput input(serial, commands);

    driver.begin();
    azDriver.begin();
    axis.begin();
    azAxis.begin();
    axis.enable(true);
    azAxis.enable(true);

    tracker.begin();
    tracker.setObserver(47.6f, -52.7f);
    tracker.setTime(1783339200UL);
    tracker.manualHome();
    axis.startGoto(model.degToSteps(45.0f), 400.0f);
    axis.update();
    input.update();
    tracker.update();

    (void)serial;
}
