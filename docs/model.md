# Local thermal model

The C model follows the equations and defaults of SameLoop UI Flutter 0.1.0.
This is a UI demo, not a validated physical model or a functional safety design.

Initial chamber: 32.4 °C; ambient: 20 °C; setpoint: 25 °C; mode: OFF. Values are
unavailable until the first sample. The GUI advances the simulator every 100 ms
using a monotonic SDL performance counter; the model integrates in <=100 ms steps.
Scheduling and display refresh are not hard real-time.

For AUTO with a valid, fresh measurement:

`requested = clamp(20 * (measurement - setpoint), 0, 100)`

Actual duty follows the request with a 0.6-second time constant. RPM follows
`40 * appliedDuty` with a 0.8-second time constant. Both exponential updates use
`1 - exp(-dt/tau)`.

`dT/dt = heat - (0.015 + 0.12 * appliedDuty/100) * (T - 20)`

Heat is normally 0.18 °C/s, or 1.2 °C/s with the heat fault selected. The temperature
uses Euler integration and is bounded to [-40, 150] °C. This proportional-only
controller has steady-state error and cannot cool below ambient. There is no RNG.

OFF, an invalid sensor or no valid sample for >=1 second requests zero duty. The
fan still takes time to stop. The UI marks stale temperature and alarm quality;
it never treats stale alarm=false as a valid all-clear. Overtemperature means
valid temperature >50 °C, without latching or hysteresis.

Disconnect is a simulated UI-link fault: control and plant continue; commands are
rejected and actual readings are hidden. The last known settings are labeled.
Reconnecting exposes current local state. Controller restart resets OFF/25 °C and
zero requested duty without resetting the plant.

## Scope and mapping

The common VSpec vocabulary maps to temperature, validity, setpoint/mode, requested
and applied duty, RPM and alarm state. This program does not load VSpec at runtime.
SensorUnavailable is shown as unavailable/stale in the combined sensor status.

No AFB, gRPC, CAN, real permissions, persistent log or real connection recovery is
implemented. UI callbacks currently call the simulation command API directly. For
a real backend, replace these commands and snapshot updates with a separate adapter;
keep the controller on the service side and do not run the simulator in parallel.

Similar equations and scenarios are provided for comparison with Flutter. Exact
cross-language numerical equivalence has not been tested against the user's
validated Flutter checkout. Benchmarking GUI frameworks requires a separate
controlled experiment; local simulator numbers do not compare service stacks.
