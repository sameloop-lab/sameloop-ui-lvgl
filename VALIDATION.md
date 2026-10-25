# Validation — 2026-10-24

Completed in a Linux x86_64 Ubuntu 24.04 environment:

- Full CMake Release build of the application and LVGL using GCC/G++ 13.3.
- LVGL v9.2.2 at the revision pinned in DEPENDENCIES.md.
- SDL2 2.30.0 upstream headers linked to the system SDL2 2.30.0 runtime.
- CTest simulation suite: PASS, including the exact one-second timeout boundary.
- CTest LVGL UI smoke suite: PASS using SDL_VIDEODRIVER=dummy.
- Software-rendered application BMP captured and visually inspected; converted
  to docs/preview.png for the README. This is an actual LVGL render.

Build infrastructure here used CMake 4.4.4 and Unix Makefiles, with explicit SDL2
header/library paths and FETCHCONTENT_SOURCE_DIR_LVGL pointing at the pinned
checkout. The LVGL download archive was retrieved separately and its SHA-256 checked.

Not yet validated: Fedora package installation, an interactive Wayland/X11 desktop
session, real mouse/keyboard input, hardware displays, ARM/ESP32 targets, external
backends or comparison benchmarks. The CI-style smoke test sends LVGL events
directly; it is not an end-to-end operating-system input test.
