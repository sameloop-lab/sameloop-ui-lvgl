# SameLoop UI — LVGL

**Bind & Seek — Same Control Loop, Different Embedded Stacks.**

A C desktop application for SameLoop Lab, using **LVGL 9.2.2 + SDL2**. This first
version reproduces the light touch-panel design and the local thermal simulator
of `sameloop-ui-flutter` 0.1.0. It is a separate repository: `sameloop-ui-lvgl`.

![Application rendered by LVGL](docs/preview.png)

## Features

- Measured temperature and accepted setpoint, adjustable from 15 to 40 °C.
- OFF/AUTO control; startup is OFF with setpoint 25 °C.
- Requested fan duty, applied duty and simulated measured RPM.
- Sensor validity, freshness, overtemperature and simulated connection state.
- Extra heat, invalid sensor, paused samples, disconnect, 55 °C injection and
  controller restart controls.
- Mouse and keyboard navigation; vertical scrolling for the lower controls.

The only backend is the **local simulator**. AFB, KUKSA, OpenHarmony and webOS are
future integrations. This version intentionally simplifies layout to a fixed
1100×900 desktop window, with vertical scrolling; it is not the responsive Flutter
layout and has not been ported to an ESP32 or a physical display.

## Build and run on Fedora

Install dependencies once:

```sh
sudo dnf install gcc gcc-c++ cmake make SDL2-devel
```

Extract `sameloop-ui-lvgl-0.1.0.zip`, then enter the **application directory**:

```sh
unzip sameloop-ui-lvgl-0.1.0.zip
cd sameloop-ui-lvgl
ls CMakeLists.txt src/main.c
```

Configure and compile:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
```

The first configure downloads LVGL automatically (approximately 68 MiB), checks its
SHA-256, and builds it inside `build/_deps`. You do not need to clone a separate
LVGL project or copy source files by hand. The selected release is a pinned
baseline, not a claim about the latest LVGL version.

Run from a graphical desktop session, **without sudo**:

```sh
./build/sameloop-ui-lvgl
```

Click AUTO to cool the chamber, use +/- to change the target, and scroll to the
simulation controls. Blue mode/fault buttons indicate the selected state. Close
the window to exit. Tab navigates focus and Enter activates a button.

For Debian/Ubuntu, the equivalent dependency installation is:

```sh
sudo apt install build-essential cmake libsdl2-dev
```

## Tests

```sh
ctest --test-dir build --output-on-failure
```

The simulation suite covers initial values, invalid commands, duty lag, the one-second
freshness boundary, recovery, alarms, disconnect rejection, restart, cooling and
repeatability. The UI smoke test invokes actual LVGL button callbacks, checks
state changes and disabled controls, and renders with SDL's dummy video driver.
It does not test a desktop compositor or real mouse hardware.

You can also run a single UI smoke test or export a BMP image without a display:

```sh
SDL_VIDEODRIVER=dummy ./build/sameloop-ui-lvgl --smoke-test
SDL_VIDEODRIVER=dummy ./build/sameloop-ui-lvgl --screenshot preview.bmp
```

The screenshot command switches to AUTO at 28 °C and advances one virtual second
for an illustrative frame, then exits. Normal interactive startup remains OFF.

## Troubleshooting

- **Missing CMakeLists.txt:** run from `sameloop-ui-lvgl`, not from the LVGL source
  directory or the umbrella repository root.
- **SDL2_INCLUDE_DIR / SDL2_LIBRARY not found:** install `SDL2-devel` on Fedora
  (`libsdl2-dev` on Debian/Ubuntu), then repeat configure.
- **No available video device:** start from your desktop terminal with DISPLAY or
  WAYLAND_DISPLAY available. Do not set SDL_VIDEODRIVER=dummy for interactive use;
  it deliberately creates no visible window.
- **Wayland/X11 driver issue:** if XWayland and DISPLAY are available, try
  `SDL_VIDEODRIVER=x11 ./build/sameloop-ui-lvgl`. Otherwise use your SDL installation's
  available native video driver.
- **Controls below the window:** scroll with the mouse wheel or focus them with Tab.
  This version uses a fixed 1100×900 window.
- **Old build configuration after moving the checkout:** configure a fresh build
  directory, e.g. `cmake -S . -B build-new -DCMAKE_BUILD_TYPE=Release`.

## Offline builds and model-only builds

For offline work, pre-download/extract the exact LVGL commit in
[DEPENDENCIES.md](DEPENDENCIES.md), then configure with an absolute path:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DFETCHCONTENT_SOURCE_DIR_LVGL=/absolute/path/to/lvgl
```

With this override, CMake uses your source tree directly; its revision is your
responsibility (the download checksum is not applied to local overrides).

To test the simulator without downloading LVGL or installing SDL:

```sh
cmake -S . -B build-model -DSAMELOOP_BUILD_UI=OFF
cmake --build build-model
ctest --test-dir build-model --output-on-failure
```

## Project organization

| Path | Purpose |
| --- | --- |
| `src/simulation.c`, `src/simulation.h` | Transport-independent plant and control model |
| `src/ui.c`, `src/ui.h` | LVGL widgets, callbacks and state display |
| `src/main.c` | SDL lifecycle, timer, smoke test and BMP capture |
| `lv_conf.h` | LVGL fonts, software renderer and SDL configuration |
| `tests/test_simulation.c` | Pure C behavior tests |
| [docs/model.md](docs/model.md) | Equations, scope and future adapter boundary |
| [VALIDATION.md](VALIDATION.md) | Actual validation and remaining limits |

There is no Flutter dependency. LVGL is statically linked into the executable;
SDL2 and normal system libraries remain runtime dependencies. Build on Fedora for
Fedora deployment. The archive contains sources, not a portable prebuilt binary.
RPM/Yocto packaging and backend integration can be added later.

Add this repository's actual URL and immutable commit to `sameloop-lab/ui/README.md`
and its delivery component index when publishing. This generation does not modify
the validated Flutter repository or create a GitHub repository.

## License and references

Original application: [MIT](LICENSE). LVGL is MIT-licensed; SDL2 uses the zlib license.
See [DEPENDENCIES.md](DEPENDENCIES.md) and `third_party/` for the pinned upstream notices.

The desktop approach follows LVGL's official PC port, which documents SDL2 and
Linux build dependencies: <https://github.com/lvgl/lv_port_pc_vscode>.
This repository uses its own application and build files, not a copy of that port.
