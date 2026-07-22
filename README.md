# GoToTracker

GoToTracker is an Arduino-compatible C++ library for non-blocking stepper-axis
control and telescope mount geometry. `MountModel` is axis-independent: use one
instance for a single axis, or separate instances when two axes have different
motors or gear ratios.

## Arduino installation

Copy this repository into the Arduino libraries directory as `GoToTracker`, or
install it as a ZIP library. Then open **File > Examples > GoToTracker >
OneAxisGoto**.

The minimal setup is:

```cpp
#include <GoToTracker.h>

ArduinoClock clock;
StepDirDriver driver(2, 3, 4); // STEP, DIR, ENABLE (active-low)
AxisController axis(clock, driver);
MountModel model(200.0f, 16.0f); // motor steps/rev, motor-to-axis ratio

void setup() {
    driver.begin();
    axis.begin();
    axis.enable(true);
    axis.startGoto(model.degToSteps(45.0f), 400.0f);
}

void loop() {
    axis.update();
}
```

`loop()` must call `update()` frequently. The controller does not allocate
memory or block while a move is in progress. Each emitted STEP pulse uses the
short `delayMicroseconds()` duration configured on `StepDirDriver`.

For a two-axis mount, construct two drivers, controllers, and mount models. The
optional `AltAzTracker` coordinates both controllers, while `SkyMath` and
`MountModel` can also be used independently.

## Desktop SIL tests

The `SilTests` executable uses `SimClock` and `FakeDriver` to reproduce the
original rate and goto tests and write deterministic comparison data. Build and
run it through CTest:

```text
cmake -S . -B build-codex -G Ninja
cmake --build build-codex
ctest --test-dir build-codex --output-on-failure
```

Results are written to `sil-data/desktop_sil_summary.csv` and
`sil-data/desktop_sil_trace.csv`. The expanded expected-versus-calculated
matrix is written to `sil-data/desktop_sil_comparisons.csv`.
