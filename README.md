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
cmake --build --preset windows-msvc-debug     # or windows-msvc-release
ctest --preset windows-msvc-debug

# Linux
cmake --preset linux-gcc      # or linux-clang
cmake --build --preset linux-gcc-debug        # or linux-gcc-release
ctest --preset linux-gcc-debug
```

The configure step (`cmake --preset ...`) is a one-time-per-machine step —
it sets up a build directory that supports *both* Debug and Release; you
pick which one at build time via `--build --preset <name>-debug` or
`<name>-release`. In Visual Studio's Open Folder view, this shows up as
the familiar Debug/Release dropdown.

## Build type (Debug vs Release)

- **Debug**: no optimization, full debug symbols, assertions active. Use
  this while developing/debugging.
- **Release**: full optimization, assertions compiled out
  (`NDEBUG` defined). Use this for anything you're timing or shipping.
- `RelWithDebInfo` is also configured (optimized, but keeps debug info) —
  handy for profiling. Build it with `cmake --build --preset
  windows-msvc-debug` swapped for a preset targeting that configuration,
  or directly: `cmake --build build/windows-msvc --config
  RelWithDebInfo`.

Don't hand-roll optimization flags (`-O3`, `/O2`, etc.) yourself — CMake's
build-type machinery already sets sensible ones per compiler; overriding
them tends to fight the toolchain rather than help it.

## SIMD / instruction set (AVX2, AVX512, ...)

Controlled by the `MATRIXLIB_ARCH` cache variable, mapped to the right
flag per compiler (`/arch:AVX512` on MSVC, `-mavx512f` etc. on GCC/Clang):

```bash
cmake --preset windows-msvc -DMATRIXLIB_ARCH=avx512
```

**This is a hardware requirement, not just a performance knob.** A binary
built with `avx512` will crash with an illegal-instruction fault on any
CPU that doesn't support AVX512 -- including plenty of current consumer
chips. Only set this if you know the machine(s) the binary will actually
run on. Leave it at the default (`none`) for anything you might run
elsewhere, and reach for AVX2 rather than AVX512 first if you do want
guaranteed vectorization -- it's far more widely supported. Proper
portability (build once, use SIMD only where available) needs runtime CPU
feature detection and dispatch, which is a separate, bigger undertaking
than a compile flag.

## Link-time optimization

Enabled automatically for Release and RelWithDebInfo builds (Debug skips
it — LTO roughly doubles link time and makes debugging less
straightforward, with no payoff while you're iterating). It lets the
compiler optimize across translation unit boundaries — e.g. inlining a
`matrixlib` function into `playground`'s `main.cpp` even though they're
compiled separately — which matters more as the library grows past a
single `.cpp` file. Nothing to configure; it's on whenever the toolchain
supports it (`check_ipo_supported` in `CMakeLists.txt` detects that).

## Other optimizations worth knowing about

Roughly in order of "worth doing soon" to "only if you need it":

- **Build type + LTO (above)** — covers the vast majority of the benefit
  for basically no cost or risk. Already set up.
- **Data alignment for SIMD** — if you start hand-vectorizing (AVX2/512
  intrinsics), `Matrix`'s backing storage should be aligned to the vector
  width (32 or 64 bytes) for full speed and to avoid some intrinsics that
  outright require it. `std::vector<double>`'s default allocator doesn't
  guarantee this — worth revisiting when you get there (a custom aligned
  allocator, or `std::aligned_alloc`/`_aligned_malloc`).
- **Fast-math** — added as an opt-in `MATRIXLIB_FAST_MATH` flag, default
  OFF. It genuinely changes floating-point results (NaN handling,
  operation reordering), not just speed, so it's a deliberate per-use
  decision, not something to leave on generally for a library other code
  will depend on.
- **Profile-guided optimization (PGO)** — compile with instrumentation,
  run representative workloads to collect real branch/hotpath data, then
  recompile using that profile. Meaningfully faster than plain `-O3` /
  `/O2` for hot code, but adds real process overhead (two build passes +
  a representative benchmark run) — worth it once you have actual
  performance-critical code and benchmarks to drive it, not before.
- **Multithreading** (OpenMP, `std::execution` parallel algorithms, or a
  thread pool) — the biggest lever of all for large matrix ops, but it's
  an algorithm/architecture decision, not a compiler flag, and out of
  scope for the build system itself.

I'd hold off on PGO and multithreading until you actually have
matrix-heavy code and benchmarks showing where time goes — optimizing
before that exists is mostly guessing.

### VS Code / any other editor on Linux

Install the CMake Tools extension (or just use the CLI commands above). The
presets file means you don't need editor-specific configuration beyond
picking a preset.

## Manually trying things out

`playground/main.cpp` is a scratch executable, separate from the unit
tests, for poking at the library by hand while developing (print
statements, quick experiments, a place to set a debugger breakpoint). It
builds automatically alongside everything else — in Visual Studio it shows
up as a runnable/debuggable target named `playground`; from the CLI:

```bash
cmake --build --preset windows-msvc --target playground
./build/windows-msvc/playground.exe
```

Set `-DMATRIXLIB_BUILD_PLAYGROUND=OFF` if you ever want to exclude it (e.g.
a CI build that only needs the library and tests).

## Notes

- **C++ standard**: targets C++23. On MSVC this currently means
  `/std:c++23preview` since MSVC has not shipped a stable, non-preview
  `/std:c++23` yet (see `CMakeLists.txt` for details and links). GCC/Clang
  use `-std=c++23` via `cxx_std_23`.
- **Testing**: uses Catch2 v3, pulled in via vcpkg's manifest mode
  (`vcpkg.json`) when the vcpkg toolchain file is active, with a
  `FetchContent` fallback so the tests still build if you ever run CMake
  without vcpkg configured.
