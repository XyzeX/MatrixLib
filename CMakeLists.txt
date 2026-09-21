cmake_minimum_required(VERSION 3.21)

project(MatrixLib
    VERSION 0.1.0
    DESCRIPTION "A small C++ matrix library"
    LANGUAGES CXX
)

# --- Options -----------------------------------------------------------
option(MATRIXLIB_BUILD_TESTS "Build unit tests" ON)
option(MATRIXLIB_BUILD_PLAYGROUND "Build the scratch dev executable" ON)

# Target instruction set for vectorized code. "none" leaves it up to the
# compiler's default baseline (safest -- runs on any machine). Anything
# else bakes in a requirement: a binary built with avx512 will crash with
# an illegal-instruction fault on a CPU that doesn't support AVX512, so
# only raise this if you know the hardware you're running on, or you're
# building specifically to benchmark/develop AVX512 code paths.
set(MATRIXLIB_ARCH "none" CACHE STRING "Target SIMD instruction set")
set_property(CACHE MATRIXLIB_ARCH PROPERTY STRINGS none avx2 avx512)

# --- Global settings -----------------------------------------------------
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake's CMAKE_CXX_STANDARD=23 currently maps to /std:c++latest on MSVC
# (which tracks C++26-in-progress), not /std:c++23preview. We want the
# actual C++23-preview flag, so the library target below sets it explicitly
# instead of relying on CMAKE_CXX_STANDARD for MSVC.
set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Library target --------------------------------------------------
add_library(matrixlib
    src/matrix.cpp
    src/shape.cpp
)

target_include_directories(matrixlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(MSVC)
    # Explicit C++23-preview flag (see note above). Switch to /std:c++23
    # once MSVC ships it as stable and you've confirmed feature parity
    # with the GCC/Clang side.
    target_compile_options(matrixlib PUBLIC /std:c++23preview)
else()
    target_compile_features(matrixlib PUBLIC cxx_std_23)
endif()

add_library(matrixlib::matrixlib ALIAS matrixlib)

# --- SIMD / instruction set -------------------------------------------
if(MATRIXLIB_ARCH STREQUAL "avx2")
    if(MSVC)
        target_compile_options(matrixlib PUBLIC /arch:AVX2)
    else()
        target_compile_options(matrixlib PUBLIC -mavx2 -mfma)
    endif()
elseif(MATRIXLIB_ARCH STREQUAL "avx512")
    if(MSVC)
        target_compile_options(matrixlib PUBLIC /arch:AVX512)
    else()
        target_compile_options(matrixlib PUBLIC -mavx512f -mavx512dq -mavx512bw -mavx512vl)
    endif()
endif()
# MATRIXLIB_ARCH=none: no extra flags, compiler's default baseline applies.

# --- Link-time optimization (Release / RelWithDebInfo only) -----------
# Maps to /GL + /LTCG on MSVC, -flto on GCC/Clang. Deliberately NOT
# enabled for Debug -- it slows the build down a lot and can make
# stepping through code in a debugger confusing, for no benefit while
# you're actively developing.
include(CheckIPOSupported)
check_ipo_supported(RESULT MATRIXLIB_IPO_SUPPORTED OUTPUT MATRIXLIB_IPO_ERROR)
if(MATRIXLIB_IPO_SUPPORTED)
    set_property(TARGET matrixlib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set_property(TARGET matrixlib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
else()
    message(STATUS "LTO not supported by this toolchain: ${MATRIXLIB_IPO_ERROR}")
endif()

# --- Optional: fast-math (OFF by default -- see warning below) --------
# Relaxes IEEE-754 floating point rules (NaN/Inf handling, operation
# reordering, denormal flushing) in exchange for speed. This changes
# *results*, not just speed -- e.g. `x != x` is no longer a reliable
# NaN check, and floating-point results become order-of-operations
# dependent in ways that can make tests flaky. For a general-purpose
# matrix library, leave this OFF; only turn it on deliberately for a
# specific numerically-tolerant workload, never as a default.
option(MATRIXLIB_FAST_MATH "Enable aggressive (IEEE-754-violating) float optimizations" OFF)
if(MATRIXLIB_FAST_MATH)
    if(MSVC)
        target_compile_options(matrixlib PRIVATE /fp:fast)
    else()
        target_compile_options(matrixlib PRIVATE -ffast-math)
    endif()
endif()

# --- Playground (scratch dev executable) ------------------------------
if(MATRIXLIB_BUILD_PLAYGROUND)
    add_executable(playground playground/main.cpp)
    target_link_libraries(playground PRIVATE matrixlib::matrixlib)
    if(MATRIXLIB_IPO_SUPPORTED)
        set_property(TARGET playground PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET playground PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    endif()
endif()

# --- Tests -----------------------------------------------------------
if(MATRIXLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
