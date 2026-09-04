# XyrisOS top-level build configuration
cmake_minimum_required(VERSION 3.20)

project(XyrisOS LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep the kernel build self-contained while allowing host-side SDK tests.
option(XYRIS_BUILD_KERNEL "Build the XyrisOS kernel" ON)
option(XYRIS_BUILD_SIMULATOR "Build the XyrisOS simulator" ON)
option(XYRIS_BUILD_SDK "Build the Xyris application SDK" ON)

if(XYRIS_BUILD_KERNEL)
    add_subdirectory(kernel)
endif()

if(XYRIS_BUILD_SIMULATOR)
    add_subdirectory(simulator)
endif()

if(XYRIS_BUILD_SDK)
    add_subdirectory(xyris-sdk)
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
