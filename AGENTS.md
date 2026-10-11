# OpenRGB Custom Patch Repository (Segilmez06/OpenRGB)

## Overview
Custom patch repository and development workspace for **OpenRGB** based on the latest stable release (**v1.0** / `release_1.0`).

## Upstream & Remotes Architecture
- **Origin (Personal Fork):** `https://github.com/Segilmez06/OpenRGB.git`
  - Default Branch: `release_1.0-patches`
- **Upstream (Official GitLab):** `https://gitlab.com/CalcProgrammer1/OpenRGB.git`
  - Canonical development repository for OpenRGB.
- **Upstream Mirror (GitHub):** `https://github.com/CalcProgrammer1/OpenRGB.git`

## Base Release Details
- **Base Tag:** `release_1.0`
- **Base Commit Hash:** `81bbe18a84c2e507006f19dd252e397e40a56bfe`
- **Release Date:** September 11, 2026
- **Working Branch:** `release_1.0-patches` (branched directly off `release_1.0`)

---

## Build System & Environment
- **Host OS:** CachyOS / Arch Linux (x86_64)
- **Qt Version:** Qt 5.15 (`/usr/bin/qmake`, `/usr/bin/qmake-qt5`)
- **Compilers:** GCC (`/usr/bin/g++`) / Clang (`/usr/bin/clang`)
- **Key Dependencies:**
  - `libusb-1.0`
  - `hidapi-hidraw`
  - `mbedtls`

### Compiling
```fish
# In repository root
mkdir -p build
cd build
qmake ../OpenRGB.pro
make -j(nproc)
```

The compiled binary will be placed at `build/openrgb`.

---

## Patching & Development Workflow
1. Keep `release_1.0-patches` as the base stable release branch.
2. Develop custom device controllers inside `Controllers/` or `RGBController/`.
3. Register new controllers in `DetectionManager.cpp` and `DetectionManager.h`.
4. Commit patches cleanly with clear descriptive messages.
5. Push changes to `origin release_1.0-patches` (or dedicated feature branches if multi-part).
