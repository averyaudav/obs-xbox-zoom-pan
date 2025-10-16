# obs-xbox-zoom-pan

An OBS dock that lets you **zoom and pan a scene item with an Xbox controller**.

- Left stick up/down → Zoom
- Right stick → Pan
- Right trigger → Reset
- Adjustable Zoom Speed, Pan Speed, Dead-zone
- Smooth damping and clamped zoom
- Settings persist between sessions
- Multi-controller support

## Build (local)

### Windows
- Easiest: use GitHub Actions (below) and download the DLL artifact.
- Local build (if you have toolchain):
  1. Install [CMake](https://cmake.org), MSVC (VS 2022 Build Tools), and optionally Ninja.
  2. Download OBS **deps kit** or point `CMAKE_PREFIX_PATH` to your OBS install’s `lib/cmake`.
  3. Configure & build:
     ```powershell
     cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\obs-deps\lib\cmake"
     cmake --build build
     ```
  4. Install:
     ```powershell
     cmake --install build --prefix "C:\Program Files\obs-studio"
     ```

### macOS
```bash
brew install cmake qt sdl2 obs
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
sudo cmake --install build --config Release --prefix "/Applications/OBS.app/Contents"
