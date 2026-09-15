cmake_minimum_required(VERSION 3.21)

project(MatrixLib
    VERSION 0.1.0
    DESCRIPTION "A small C++ matrix library"
    LANGUAGES CXX
)

# --- Options -----------------------------------------------------------
option(MATRIXLIB_BUILD_TESTS "Build unit tests" ON)

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

# --- Tests -----------------------------------------------------------
if(MATRIXLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
