#include <GoToTracker.h>

void compileArduinoSurface(Stream& serial) {
    ArduinoClock clock;
    StepDirDriver driver(2U, 3U, 4U);
    AxisController axis(clock, driver);
    MountModel model(200.0f, 16.0f);

    driver.begin();
    axis.begin();
    axis.enable(true);
    axis.startGoto(model.degToSteps(45.0f), 400.0f);
    axis.update();

    (void)serial;
}
