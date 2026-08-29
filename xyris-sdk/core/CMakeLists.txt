cmake_minimum_required(VERSION 3.20)

project(XyrisSDKCore C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(xyris-sdk-core STATIC
    src/core.c
)

target_include_directories(xyris-sdk-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)

target_compile_options(xyris-sdk-core PRIVATE
    -Wall
    -Wextra
    -Werror
    -ffreestanding
    -fno-stack-protector
    -fno-stack-check
    -fno-pic
    -fno-pie
    -mno-red-zone
)

target_compile_definitions(xyris-sdk-core
    PUBLIC
        XYRIS_SDK_CORE=1
)

add_executable(xyris-sdk-core-c-header-test
    tests/core_header_test.c
)

target_link_libraries(xyris-sdk-core-c-header-test PRIVATE xyris-sdk-core)

add_executable(xyris-sdk-core-cpp-header-test
    tests/core_header_test.cpp
)

target_link_libraries(xyris-sdk-core-cpp-header-test PRIVATE xyris-sdk-core)

install(TARGETS xyris-sdk-core
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)
