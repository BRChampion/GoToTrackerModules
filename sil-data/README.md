# Desktop SIL baseline

Run the deterministic desktop software-in-the-loop tests with:

```text
cmake -S . -B build-codex -G Ninja
cmake --build build-codex
ctest --test-dir build-codex --output-on-failure
```

The test writes two files here:

- `desktop_sil_summary.csv` contains one result per scenario.
- `desktop_sil_trace.csv` contains one row for every simulated STEP pulse.
- `desktop_sil_comparisons.csv` contains the expanded coordinate, wrapping,
  timing, and end-to-end matrix with expected and calculated results,
  tolerances, absolute errors, and pass/fail status.

The clock advances in deterministic 1 ms increments, so these values represent
controller behavior without operating-system scheduling jitter. For comparison
on Arduino, record elapsed microseconds, controller position, physical or
commanded direction, and pulse count using the same CSV columns.

Astronomical coordinate expectations use analytically known cardinal cases.
The Vega and Polaris end-to-end expectations use fixed double-precision
reference results for Unix time `1783353600`, latitude `47.6` degrees, and
longitude `-52.7` degrees. The implementation under test calculates these
paths using its normal single-precision embedded code.

The fixed references were calculated independently from the embedded path:

1. Convert Unix time to Julian date with `JD = unix / 86400 + 2440587.5`.
2. Calculate double-precision GMST using the standard J2000 polynomial.
3. Add longitude to obtain local sidereal time.
4. Apply the altitude and quadrant-safe `atan2` azimuth equations.
5. Round the resulting angles using the configured 3200 steps per revolution.

The reference local sidereal time is `111.9483679258` degrees. Reference
horizontal coordinates are:

| Target | Altitude (deg) | Azimuth (deg) |
|---|---:|---:|
| Vega | -2.8767321101 | 9.8908106895 |
| Polaris | 47.7981093169 | 358.9469510188 |

The first two scenarios reproduce the original `main.cpp` tests from `master`.
Despite its old comment mentioning step 1000, the original goto command targets
step -500.
