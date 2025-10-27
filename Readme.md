# EL_Draw_3D_open

![Demo image](pic/image.png)

A small demo that runs on a RP2040 (Raspberry Pi Pico family) platform and draws 3D mesh content on an EL display using the project's `el` display library.

This repository contains a 3D mesh demo (`main_3d_demo.c`) that demonstrates multicore drawing, a simple mesh renderer, and buffer swapping for an EL screen.

## Features

- Runs on RP2040 using the Pico SDK
- Multicore rendering (core1 runs the renderer)
- Mesh drawing demo (see `draw_mesh.h` / `draw_mesh.c`)
- Simple memory/stack diagnostics and watchdog integration

## Hardware

- Raspberry Pi Pico or compatible RP2040 board
- EL (Electroluminescent) display and driver supported by the `el` library included in this project
- USB cable for flashing

Wiring depends on your EL driver and board; the code assumes `el` library initialization via `el_start()` and uses the Pico's peripherals for timing and GPIO.

## Prerequisites

- CMake and Make
- Pico SDK (this repo already includes a `pico_sdk` subtree)
- A suitable toolchain for cross-building RP2040 targets (e.g. `arm-none-eabi-gcc`) if building outside the provided environment

On macOS you can install dependencies with Homebrew, for example:

```sh
# install cmake and arm toolchain if needed
brew install cmake arm-none-eabi-gcc
```

## Build

From the repository root:

```sh
mkdir -p build
cd build
cmake ..
make
```

If successful, a UF2 or binary will be available under `build/` (for example `eldemo.uf2`).

## Flashing

- Copy the generated `.uf2` file (for example `build/eldemo.uf2`) to your Pico when it is in bootloader mode (double-tap the BOOTSEL button on the Pico and copy the UF2 to the mounted USB mass storage device).
- Alternatively, use `picotool` if you prefer command-line flashing.

## Run

After flashing, reset or power-cycle the board. The demo will start automatically and use the EL display. The on-board LED is toggled during startup and the program prints debug messages to the standard USB serial output.

## Important files

- `main_3d_demo.c` — demo entry, multicore orchestration and main loop
- `draw_mesh.h` / related files — mesh drawing implementation
- `el.h` / `el.c` — EL display driver and helpers

## Notes

- This demo configures the system clock and uses higher voltages to reach higher CPU frequencies; modify clock and voltage settings to fit your hardware and thermal budget.
- There is a small memory/stack diagnostic helper in `main_3d_demo.c`.

## License

Check for a `LICENSE` file in this repository. If none exists, contact the project owner or maintainers for licensing details.

## Contact

Create an issue on the repository if you need help or want to report a bug.

