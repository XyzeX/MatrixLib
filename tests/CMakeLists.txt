# Try vcpkg's installed Catch2 first (works via the CMAKE_TOOLCHAIN_FILE
# set in CMakePresets.json). If it's not found -- e.g. you're on the Linux
# box without vcpkg set up -- fall back to fetching it directly, so `cmake
# --build` still works with nothing but a compiler and CMake installed.
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()

add_executable(matrixlib_tests
    test_matrix.cpp
)

target_link_libraries(matrixlib_tests
    PRIVATE
        matrixlib
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(matrixlib_tests)
