# MatrixLib

A small C++ matrix library, built with CMake, vcpkg, and Catch2.

## Prerequisites

- CMake >= 3.21
- Ninja (or any generator you like — presets below assume Ninja)
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set as an
  environment variable pointing at your vcpkg checkout
- Windows: Visual Studio 2022 (17.13+) with the C++ workload, or the
  standalone Build Tools
- Linux: GCC 13+ or Clang 17+ (for reasonable C++23 language support)

## Building

### Visual Studio (Windows)

Open the folder in Visual Studio via **File > Open > Folder...** and point
it at this directory. VS detects `CMakePresets.json` automatically and
lists the `windows-msvc` preset in the configuration dropdown. Build and
run tests from there, or from **Test Explorer** once configured.

### Command line (Windows or Linux)

```bash
# Windows (from a Developer PowerShell / Command Prompt)
cmake --preset windows-msvc
cmake --build --preset windows-msvc
ctest --preset windows-msvc

# Linux
cmake --preset linux-gcc      # or linux-clang
cmake --build --preset linux-gcc
ctest --preset linux-gcc
```

### VS Code / any other editor on Linux

Install the CMake Tools extension (or just use the CLI commands above). The
presets file means you don't need editor-specific configuration beyond
picking a preset.

## Notes

- **C++ standard**: targets C++23. On MSVC this currently means
  `/std:c++23preview` since MSVC has not shipped a stable, non-preview
  `/std:c++23` yet (see `CMakeLists.txt` for details and links). GCC/Clang
  use `-std=c++23` via `cxx_std_23`.
- **Testing**: uses Catch2 v3, pulled in via vcpkg's manifest mode
  (`vcpkg.json`) when the vcpkg toolchain file is active, with a
  `FetchContent` fallback so the tests still build if you ever run CMake
  without vcpkg configured.
