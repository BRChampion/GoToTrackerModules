# TODO - Arduino Optimization Branch

- [x] Add continuous two-axis correction for Alt/Az tracking
- [x] Add basic serial monitor command parser
- [x] Add manual home and one-star sync commands
- [x] Add Arduino `Serial` line-buffer adapter
- [ ] Add current-time command or RTC/GPS time source
- [ ] Add persistent calibration storage for Arduino EEPROM

## Arduino software-in-the-loop tests

- [ ] Add an Arduino `CountingDriver` that records direction, pulse count, and
  pulse timestamps without driving GPIO.
- [ ] Port the desktop coordinate, angle-wrapping, rate, goto, rollover, and
  end-to-end catalog cases to an Arduino test sketch using `ArduinoClock`.
- [ ] Print Arduino results as CSV with the same expected, calculated,
  tolerance, error, and pass/fail columns as the desktop SIL output.
- [ ] Buffer timing measurements during each test and print them afterward so
  `Serial` output does not perturb the scheduler being measured.
- [ ] Exercise one-axis and simulated two-axis `AltAzTracker` configurations.

## TB6600 / NEMA 17 electrical tests

- [x] Measure and record STEP pulse width, pulse period, achieved rate,
  direction setup/hold timing, and ENABLE polarity.
- [x] Repeat the measurement across several configured pulse widths and step
  rates, including a rate near the practical loop-frequency limit.
- [x] Confirm TB6600 DIP switch settings for microstepping and current limit,
  then copy the final values into the test notes.
- [ ] Run unloaded NEMA 17 movement tests at conservative rates and record the
  fastest reliable start/stop rate before adding acceleration control.
- [ ] Verify commanded positive and negative step directions match the chosen
  altitude and azimuth sign conventions.

## One-motor target-to-motion hardware tests

- [x] Build a single-axis hardware test sketch that can run either the
  altitude or azimuth axis logic using the same NEMA 17 and TB6600.
- [x] Add serial commands or compile-time constants for selecting the axis
  under test: `alt` or `az`.
- [x] Compile the hardware test sketch for the Arduino Mega 2560 with
  `arduino-cli`.
- [ ] For azimuth testing, mark the motor shaft/coupler, start from a known
  physical north reference, run `home`, feed a target, run `goto`, and compare
  the final physical azimuth with the expected azimuth from `status`.
- [ ] Repeat the azimuth test with targets on both sides of north to verify
  shortest-path wrapping across 0/360 degrees.
- [ ] For altitude testing, mark the motor shaft/coupler, start from a known
  physical level or zero-altitude reference, run `home`, feed a target, run
  `goto`, and compare the final physical altitude with the expected altitude
  from `status`.
- [ ] Repeat the altitude test with positive and negative target altitudes if
  the bench setup allows safe travel in both directions.
- [ ] Record for each test: target name, time source, observer latitude and
  longitude, expected Alt/Az, expected step target, starting physical mark,
  final physical mark, observed error, microstep setting, and gear ratio.
- [ ] Repeat one azimuth and one altitude test after `cal one-star` to confirm
  manual sync does not break the target-to-motor mapping.
- [ ] Define an acceptable bench tolerance in degrees or motor-shaft marks
  before treating the hardware test as passed.

## Documentation and release readiness

- [x] Document Arduino CLI compilation, upload, serial commands, and the
  one-motor test workflow in the README.
- [ ] Replace the fixed startup Unix time with a current-time command or an
  RTC/GPS source before live sky tests.
- [ ] Document the final Mega pin assignments, TB6600 settings, microstepping,
  gear ratios, maximum reliable rates, and axis sign conventions.
- [ ] Run and record at least one complete altitude and one complete azimuth
  target-to-motion acceptance test.
