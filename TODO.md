# TODO - Arduino Optimization Branch

- [x] Add continuous two-axis correction for Alt/Az tracking
- [x] Add basic serial monitor command parser
- [x] Add Arduino `Serial` line-buffer adapter
- [ ] Add current-time command or RTC/GPS time source

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

## STEP/DIR electrical tests

- [ ] Add a slow, repetitive STEP/DIR oscilloscope test sketch that does not
  require a connected motor or driver load.
- [ ] Measure and record STEP pulse width, pulse period, achieved rate,
  direction setup/hold timing, and ENABLE polarity.
- [ ] Repeat the measurement across several configured pulse widths and step
  rates, including a rate near the practical loop-frequency limit.

## 28BYJ-48 and ULN2003 bench tests

- [ ] Implement a separate non-blocking `Driver` for the four-coil ULN2003
  interface; do not route it through the STEP/DIR driver.
- [ ] Support a configurable four-pin coil order and an eight-state half-step
  sequence for the 28BYJ-48.
- [ ] De-energize all coils when the axis is disabled or stopped, with an
  option to hold position when required.
- [ ] Test forward/reverse sequencing, exact step counts, rate timing, stop,
  resume, and `micros()` rollover before connecting the motor.
- [ ] Run conservative unloaded motor tests and document the usable maximum
  start rate before adding acceleration control.
