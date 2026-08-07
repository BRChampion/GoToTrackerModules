# GoToTracker

GoToTracker is an Arduino-compatible C++ library for non-blocking STEP/DIR
motor control and Alt/Az telescope tracking. It converts catalog RA/Dec targets
to local altitude and azimuth, coordinates two independent axes, chooses the
shortest azimuth path, and provides manual home and one-star synchronization.

The code targets the Arduino Mega 2560 and avoids dynamic allocation and the
C++ standard library on the Arduino path. Time is kept as integer Unix seconds;
the sidereal-time calculation splits elapsed time into whole days and seconds
before using single-precision arithmetic.

## Arduino Mega quick start

The one-motor bench sketch is
`examples/AltAzSerialHardwareTest/AltAzSerialHardwareTest.ino`. Before compiling,
edit its hardware and observer settings:

- `TestAxisMode`: `AxisAz` or `AxisAlt`
- `StepPin`, `DirectionPin`, and `EnablePin`
- `EnableActiveLow` and `StepPulseMicros` for the TB6600
- motor steps per revolution and motor-to-axis ratios
- observer latitude, longitude, and the fixed UTC Unix test time

From the repository root, compile with Arduino CLI:

```powershell
arduino-cli.exe compile --fqbn arduino:avr:mega --library . examples\AltAzSerialHardwareTest
```

Connect the Mega, find its port, and upload the sketch:

```powershell
arduino-cli.exe board list
arduino-cli.exe upload -p COM3 --fqbn arduino:avr:mega --library . examples\AltAzSerialHardwareTest
```

Replace `COM3` with the port reported for the board. Open the serial monitor at
`115200` baud with line endings enabled.

The smaller `examples/OneAxisGoto/OneAxisGoto.ino` example demonstrates the
minimum single-axis setup:

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

`loop()` must call `update()` frequently. Each call checks whether a motor step
is due, emits at most one step, and returns so serial commands and both axes stay
responsive. Only the configured STEP pulse width uses a short
`delayMicroseconds()` call.

## Serial commands

| Command | Action |
| --- | --- |
| `help` | Print the available commands |
| `status` | Print target, tracking/calibration state, positions, and targets |
| `target <index\|name>` | Select `Vega`, `Polaris`, or `Betelgeuse` |
| `home` | Set the current physical Alt/Az position as zero |
| `sync` | Synchronize the current position to the selected star |
| `calibrate` | Alias for `sync` |
| `cal one-star` | Explicit one-star synchronization command |
| `nudge alt\|az <steps>` | Move one axis by a signed number of steps |
| `goto` | Slew both logical axes to the selected target |
| `track on\|off` | Enable or disable periodic target correction |
| `stop` | Stop both axes and disable tracking |

Target names are case-sensitive. A repeatable uncalibrated bench test is:

```text
home
target Vega
goto
status
stop
```

For one-star calibration, physically center a selected star using signed
`nudge` commands, then run `sync` or `cal one-star`. Synchronization requires a
selected target and treats the current physical position as that target's
calculated Alt/Az position.

The hardware sketch connects the selected logical axis to the real TB6600 and
uses a no-output driver for the other axis. This allows the same motor and
driver to test the complete altitude path and then the complete azimuth path.

## Desktop simulation and tests

The desktop build contains SIL coverage for rate and goto motion, timer
rollover, coordinate conversion, shortest-path azimuth motion, catalog targets,
and manual home/one-star calibration.

```powershell
cmake -S . -B build-codex
cmake --build build-codex
ctest --test-dir build-codex --output-on-failure
```

For a terminal demonstration of shortest-path azimuth movement:

```powershell
.\build-codex\ShortestPathDemo.exe
```

For a two-axis mount, construct two drivers, controllers, and mount models. The
optional `AltAzTracker` coordinates both controllers, while `SkyMath` and
`MountModel` can also be used independently.
