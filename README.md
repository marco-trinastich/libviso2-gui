# LibViso2 GUI

LibViso2 GUI is a native **Windows desktop application** that provides an interactive graphical front-end for **stereo visual odometry**, built on top of the well-known **[LibViso2](http://www.cvlibs.net/software/libviso/)** library. It processes sequences of rectified **stereo image pairs**, estimates the camera's frame-by-frame egomotion, and renders both the live image stream and the reconstructed **trajectory** in real time — offering an approachable playground to experiment with and visualize visual odometry.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Features](#features)
3. [Screenshots](#screenshots)
4. [Environment Setup](#environment-setup)
   - [Prerequisites](#prerequisites)
   - [Clone the Repository](#clone-the-repository)
   - [Dataset](#dataset)
   - [Setup Steps](#setup-steps)
   - [Configuration](#configuration)
5. [Usage](#usage)
   - [How to Run](#how-to-run)
   - [Parameters](#parameters)
6. [Development](#development)
   - [Project Structure](#project-structure)
   - [Target platforms](#target-platforms)
   - [Debugging](#debugging)
   - [Improvements](#improvements)
7. [Extras](#extras)
   - [FAQs](#faqs)
8. [Acknowledgments](#acknowledgments)
9. [Support](#support)
10. [License](#license)

## Project Overview

LibViso2 GUI wraps the C++ **LibViso2** (Library for Visual Odometry 2) engine in a lightweight Win32 application, turning a command-line computer-vision library into a visual, real-time tool.

Given a folder of grayscale stereo image pairs (a left and a right camera view per frame), the application:

- loads each stereo pair and feeds it to the LibViso2 stereo visual-odometry estimator;
- computes the incremental camera motion and accumulates the global **pose** (position and orientation) across the sequence;
- displays the current frame in a **video window** and plots the accumulated **camera trajectory** together with live processing statistics (matches, inliers, current frame) in a separate **info window**.

The project is intended for **research, learning, and experimentation** in visual odometry, giving an immediate visual feel for how a stereo VO pipeline behaves on real data. It ships with a sample **KITTI** stereo sequence so it can be run out of the box.

## Features

### Stereo Visual Odometry

- Real-time, frame-by-frame egomotion estimation powered by **LibViso2** (`VisualOdometryStereo`).
- Accumulates the global 4×4 camera pose and derives the 3D position `(x, y, z)` per frame.
- Preconfigured camera **calibration** (focal length, principal point, baseline) for the bundled KITTI sequence.

### Real-Time Visualization

- **Video window** — renders the current stereo frame using **Direct2D** + **Windows Imaging Component (WIC)**.
- **Info window** — drawn with **GDI+**, showing:
  - the active visual-odometry parameters;
  - live statistics (number of matches, number of inliers, current frame, image file name);
  - a **trajectory graph** (top-down X/Z view plus a Y/frame plot).

### Responsive, Multi-Threaded Design

- The visual-odometry pipeline runs on a **dedicated worker thread**, keeping the GUI responsive while frames are processed.
- Optional attached **debug console** for real-time textual logging of per-frame stats.

### Standalone & Portable

- Self-contained: precompiled dependencies (**LibViso2**, **libpng**, **zlib**) are bundled under `libs/`.
- Reads standard grayscale **PNG** image pairs via **libpng / png++**.
- Configurable dataset path and image-name prefixes via command-line arguments.

## Screenshots

![LibViso2 GUI](assets/screenshots/LibViso2%20GUI.jpg)

_The application opens two windows: a video window showing the current stereo frame and an info window plotting the reconstructed trajectory and live statistics._

## Environment Setup

### Prerequisites

- **Windows 10 or 11** (the application relies on Win32, Direct2D, GDI+, and WIC). Builds and runs on **x86 (Win32)**, **x64**, and **ARM64**.
- **Visual Studio 2022** with the **"Desktop development with C++"** workload and the **C++ CMake tools** component (this provides both the MSVC toolchain and CMake, which the build scripts locate automatically via `vswhere`). For x64/ARM64 native builds make sure the matching build tools are installed in the VS installer.
- **Visual C++ 2015–2022 Redistributable** on the machine that *runs* the app — it provides `VCRUNTIME140.dll`. (The Universal CRT itself ships with Windows 10/11, so no other runtime is bundled.)
- _(Optional)_ **Visual Studio Code** with the C/C++ and CMake Tools extensions, for the preconfigured build/debug tasks.

> The **Win32** dependency binaries (LibViso2, libpng, zlib) are committed under `libs/bin/Win32` + `libs/lib/Win32`, so a default Win32 build needs no dependency setup. For **x64/ARM64**, the dependencies are compiled from source on demand — see [Target platforms](#target-platforms).

### Clone the Repository

```bash
git clone https://github.com/marco-trinastich/libviso2-gui.git
cd libviso2-gui
```

### Dataset

The image dataset is **not included in the repository** (it is excluded via `.gitignore` to keep the repo lightweight). The default sample sequence is **`2010_03_09_drive_0019`** — the standard demonstration sequence distributed with **LibViso2**, recorded as part of the Karlsruhe / KITTI data.

To obtain it:

1. Download a sequence from the official **Karlsruhe Dataset** page: **<https://www.cvlibs.net/datasets/karlsruhe_sequences/>**, which lists all available sequences. Use `2010_03_09_drive_0019` to match the default calibration out of the box, or pick **any other sequence** you like — each one ships with its own calibration file that you can use to recalibrate the app (see the note below). (This dataset is also referenced in `libs/include/libviso2/readme.txt`.)
2. Extract it anywhere you like. Dropping it into the default path `assets/datasets/libviso2/2010_03_09_drive_0019/` lets you run the app with no further changes; otherwise, just point the app at your chosen folder — either update `libviso2.imagesPath` in `.vscode/settings.json`, or pass the path as the `<imagesPath>` argument when launching (see [Usage](#usage)).

   ```
   <your-folder>/
   ├── I1_000000.png   # left frames
   ├── I2_000000.png   # right frames
   └── ...
   ```

3. The images already follow the expected `I1_` / `I2_` (left / right) naming convention, so **no renaming is required**.

> **Calibration:** the default parameters in `src/viso/viso_core.cpp` (`SetupVOParams`) are tuned for `2010_03_09_drive_0019`. To use a different Karlsruhe sequence — or your own stereo data — point `<imagesPath>` at its folder and update the calibration (focal length `f`, principal point `cu`/`cv`, baseline `base`) to match. Each Karlsruhe sequence provides these values in the calibration file (`calib.txt`) shipped alongside it.

### Setup Steps

The project uses **CMake** with the Visual Studio generator. The **target platform**
(`Win32` / `x64` / `ARM64`) is selectable and defaults to `Win32` — see
[Target platforms](#target-platforms). You can build from VS Code or the command line.

**Option A — VS Code (recommended):**

1. Open the project folder in VS Code.
2. _(x64/ARM64 only)_ Set `libviso2.targetPlatform` in `.vscode/settings.json`, then run **`(Dependencies) Build`** once to compile the third-party libs for that target. (Win32 is committed, so this is a no-op.)
3. Run the task **`(LibViso2 App) Configure`** (Terminal → Run Task…) to generate the CMake build files.
4. Run **`(LibViso2 App) Build [Release]`** (or `[Debug]`) to compile the application.
   - `(LibViso2 App) Clean and Rebuild [Debug]/[Release]` perform a clean rebuild in one step.
   - Switching platform is safe: the configure step auto-cleans a stale cache/output from a previous target.

**Option B — Command line:**

```powershell
# From the project root (replace Win32 with x64 or ARM64 as desired)
cmake -S . -B build -A Win32 -T host=ARM64
cmake --build build --config Release
```

The resulting executable is produced at `build/Release/libviso2gui.exe` (or `build/Debug/libviso2gui.exe`).

### Configuration

- **Dataset & run arguments (VS Code):** defined in `.vscode/settings.json`:

  ```json
  "libviso2.imagesPath": "./assets/datasets/libviso2/2010_03_09_drive_0019",
  "libviso2.imagesPrefixLeft": "/p1=I1",
  "libviso2.imagesPrefixRight": "/p2=I2"
  ```

  These values are injected into the launch/debug arguments.

- **Camera calibration:** the intrinsic/extrinsic parameters (focal length, principal point, baseline) are set in `src/viso/viso_core.cpp` (`SetupVOParams`). They are tuned for the bundled KITTI sequence `2010_03_09_drive_0019`; adjust them if you use a different camera/dataset.

- **Application settings:** compile-time options (console logging, image extension, default prefixes, GUI layout) live in `src/core/models/settings.h`.

## Usage

### How to Run

The app expects a dataset folder containing grayscale PNG stereo pairs, where left and right frames share a common numeric suffix and differ only by a prefix (default `I1_` for left, `I2_` for right).

The repository includes a ready-to-use sample sequence, so you can launch it immediately with the provided batch scripts:

```powershell
# Release build
run_release.bat

# Debug build
run_debug.bat
```

Each script simply invokes the executable against the bundled dataset, e.g.:

```powershell
build\Release\libviso2gui.exe ./assets/datasets/libviso2/2010_03_09_drive_0019 /p1=I1 /p2=I2
```

### Parameters

The executable is driven by command-line arguments:

| Argument       | Description                                                                 | Default |
| -------------- | --------------------------------------------------------------------------- | ------- |
| `<imagesPath>` | **(Positional, required)** Path to the folder containing the stereo images. | —       |
| `/p1=<prefix>` | Filename prefix identifying the **left** images.                            | `I1_`   |
| `/p2=<prefix>` | Filename prefix identifying the **right** images.                           | `I2_`   |
| `/h`           | Show help.                                                                  | —       |

Images must be **PNG** grayscale files; left/right frames are paired by matching the portion of the filename that follows their respective prefix.

## Development

### Project Structure

```
├── src/
│   ├── core/          # Application bootstrap, handlers, models, utilities
│   │   ├── entrypoint.cpp   # WinMain + message loop
│   │   ├── handlers/        # Core (processing) and GUI handlers
│   │   ├── models/          # Settings, application State, data models
│   │   └── utils/           # Threading, console, image & PNG helpers
│   ├── gui/           # Win32 windows (video window, info window, base window)
│   └── viso/          # Visual-odometry orchestration over LibViso2
├── libs/              # Dependency sources + per-platform prebuilt binaries (bin|lib/<platform>)
├── assets/
│   ├── datasets/      # Stereo sequences (git-ignored, downloaded separately)
│   └── screenshots/   # Images used by this README
├── .vscode/           # Build/debug tasks, launch configs, helper scripts
└── CMakeLists.txt     # Top-level build definition
```

### Target platforms

The app builds for **`Win32` (x86)**, **`x64`**, and **`ARM64`**. The target is chosen at configure time:

- **VS Code:** set `"libviso2.targetPlatform"` in `.vscode/settings.json` (`Win32` | `x64` | `ARM64`).
- **CLI:** pass `-A <platform>` to CMake (or `-Platform` to `.vscode/scripts/cmake-configure.ps1`).

Dependency artifacts live per platform under `libs/bin/<platform>` and `libs/lib/<platform>`:

- **Win32** binaries are **committed** (zero-friction default build).
- **x64 / ARM64** are built from source on demand by **`(Dependencies) Build`** (skip-if-present) and are git-ignored. **`(Dependencies) Rebuild`** forces a clean rebuild of all three libs (zlib → libpng → libviso2) for the current target and redeploys them.

All targets link the **Universal CRT** — no VS2013/CRT redistributable is bundled. On **ARM64**, libviso2's SSE2/SSE3 intrinsics are bridged to NEON via a vendored `sse2neon.h` (in `libs/include/libviso2/`), enabled automatically for the ARM64 target (which also needs MSVC's `/Zc:preprocessor`, set by the build).

> If you modify libviso2's own sources under `libs/include/libviso2/`, run **`(Dependencies) Rebuild`** to recompile and redeploy `libviso2.dll` / `.lib` for the current target.

### Debugging

The `.vscode/launch.json` file defines ready-to-use debug configurations (`cppvsdbg`) for both Debug and Release, each optionally chained to a build or clean-and-rebuild pre-launch task. Launch arguments are taken from `.vscode/settings.json`.

### Improvements

Potential future enhancements:

- **In-app configuration UI** for calibration and dataset selection (currently compile-time / CLI).
- **Live camera input** in addition to offline image sequences.
- **Monocular odometry** mode (LibViso2 also provides a monocular estimator).
- **Automated tests** for the image-loading and parameter-parsing utilities.

## Extras

### FAQs

**Q: The app opens but no images appear / it reports a path error.**
A: Ensure `<imagesPath>` points to a folder of PNG stereo pairs and that the `/p1=` and `/p2=` prefixes match your filenames. Left and right images must exist in equal numbers.

**Q: Can I use my own dataset?**
A: Yes — provide grayscale PNG stereo pairs and update the calibration parameters in `src/viso/viso_core.cpp` to match your camera.

**Q: Which CPU architectures are supported?**
A: `Win32` (x86), `x64`, and `ARM64` — selectable via `libviso2.targetPlatform` (VS Code) or `-A` (CLI). Win32 dependency binaries are committed; x64/ARM64 are built from source on demand (see [Target platforms](#target-platforms)).

**Q: The app fails to start with a missing `VCRUNTIME140.dll` error.**
A: Install the **Visual C++ 2015–2022 Redistributable** for your architecture. The Universal CRT ships with Windows 10/11, but `VCRUNTIME140.dll` comes with that redistributable.

## Acknowledgments

This project builds upon excellent open-source work:

- **[LibViso2](http://www.cvlibs.net/software/libviso/)** — the stereo/monocular visual-odometry library by Andreas Geiger et al. (Karlsruhe Institute of Technology).
- **[libpng](http://www.libpng.org/)** and **[png++](https://www.nongnu.org/pngpp/)** — PNG image reading.
- **[zlib](https://zlib.net/)** — compression backend for libpng.
- Sample data from the **[Karlsruhe Dataset sequences](https://www.cvlibs.net/datasets/karlsruhe_sequences/)** (Karlsruhe Institute of Technology).

## Support

If you find this project useful, consider supporting its development:

- ⭐ Star the repository to show your appreciation.
- 💬 Share feedback or suggestions by opening an issue.
- ☕ [Buy me a coffee](https://buymeacoffee.com/mtmarco87) to support future updates and improvements.
- 🔵 BTC Address: `bc1qzy6e99pkeq00rsx8jptx93jv56s9ak2lz32e2d`
- 🟣 ETH Address: `0x38cf74ED056fF994342941372F8ffC5C45E6cF21`

## License

This project is licensed under the **GNU General Public License v3.0**. See the [LICENSE](LICENSE) file for the full text.

Copyright (c) 2014–2025 Marco Trinastich.
