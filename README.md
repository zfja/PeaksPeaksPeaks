# PeaksPeaksPeaks

[![Doxygen GitHub Pages](https://github.com/zfja/PeaksPeaksPeaks/actions/workflows/doxygen.yml/badge.svg)](https://github.com/zfja/PeaksPeaksPeaks/actions/workflows/doxygen.yml)

Qt app for viewing spectrum data from `.txt` files, smoothing with Savitzky–Golay, and measuring up to two peaks.

Docs: **https://zfja.github.io/PeaksPeaksPeaks/**

## Download

Pre-built packages: **[Releases](https://github.com/zfja/PeaksPeaksPeaks/releases/latest)**

| Platform | File |
|----------|------|
| macOS | `PeaksPeaksPeaks-macos.dmg` — open the DMG, drag the app to Applications |
| Windows | `PeaksPeaksPeaks-windows.zip` — unzip and run `PeaksPeaksPeaks.exe` |

Releases are published when a version tag is pushed (e.g. `v0.1.0`). You can also grab the latest build from the **Actions** tab (workflow artifacts).

## Build

Requires CMake (>= 3.16) and Qt 5/6 with **Charts**.

```bash
cmake -B build
cmake --build build
```

The executable is in `build/`.

## Usage

- **Choose folder…** — pick a folder with `.txt` spectra (remembered between runs)
- **Up/Down** — switch spectra in the list
- **Mouse on chart** — place peak center
- **Left Mouse Click** — confirm center, then set width
- **Left/Right** — adjust peak width
- **Enter** — confirm width
- **R** — go back one step
- **Ctrl/Cmd + R** — clear measurement for the current file
- **Save** / **Ctrl/Cmd + S** — save chart as PNG

Results go into a `peakspeakspeaks` subfolder inside your data folder:

- `data.csv` — peak measurements
- PNG exports (single or **Save all** from Settings)

! data is saved to `data.csv` file when the user switches spectra in the list !

## Settings

Colors, axis titles, and Savitzky–Golay parameters. Changes are saved globally.

## Author

Zofia Tryznowska ([@zfja](https://github.com/zfja))
