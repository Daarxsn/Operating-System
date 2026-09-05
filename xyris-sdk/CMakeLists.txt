cmake_minimum_required(VERSION 3.20)

project(XyrisSDK C CXX)

include(CTest)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core)
add_subdirectory(process)
add_subdirectory(thread)
add_subdirectory(memory)
add_subdirectory(filesystem)
add_subdirectory(ipc)
add_subdirectory(events)
add_subdirectory(timers)
add_subdirectory(devices)
add_subdirectory(networking)
add_subdirectory(security)

# One public target for application developers. Individual module targets remain
# available for projects that want a narrower dependency surface.
add_library(xyris-sdk INTERFACE)
add_library(Xyris::SDK ALIAS xyris-sdk)
target_include_directories(xyris-sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/process/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thread/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/memory/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filesystem/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ipc/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/events/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/timers/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/devices/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/networking/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/security/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../abi/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(xyris-sdk INTERFACE
    xyris-sdk-core
    xyris-sdk-process
    xyris-sdk-thread
    xyris-sdk-memory
    xyris-sdk-filesystem
    xyris-sdk-ipc
    xyris-sdk-events
    xyris-sdk-timers
    xyris-sdk-devices
    xyris-sdk-networking
    xyris-sdk-security
)

add_executable(xyris-sdk-c-header-test tests/sdk_header_test.c)
target_link_libraries(xyris-sdk-c-header-test PRIVATE xyris-sdk)
target_link_options(xyris-sdk-c-header-test PRIVATE -no-pie)

add_executable(xyris-sdk-cpp-header-test tests/sdk_header_test.cpp)
target_link_libraries(xyris-sdk-cpp-header-test PRIVATE xyris-sdk)
target_link_options(xyris-sdk-cpp-header-test PRIVATE -no-pie)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-c-header-test COMMAND xyris-sdk-c-header-test)
    add_test(NAME xyris-sdk-cpp-header-test COMMAND xyris-sdk-cpp-header-test)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME xyris-dev-cli-test COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tests/dev-tools/test_xyris_dev.py)
        add_test(NAME xyris-documentation-test COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tests/documentation/validate_docs.py)
    endif()
endif()

include(CMakePackageConfigHelpers)

install(TARGETS
    xyris-sdk
    xyris-sdk-core
    xyris-sdk-process
    xyris-sdk-thread
    xyris-sdk-memory
    xyris-sdk-filesystem
    xyris-sdk-ipc
    xyris-sdk-events
    xyris-sdk-timers
    xyris-sdk-devices
    xyris-sdk-networking
    xyris-sdk-security
    EXPORT XyrisSDKTargets
)

install(FILES core/include/xyris/sdk.h DESTINATION include/xyris)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/xyris/XyrisApplication.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/../toolchain/xyris-user.cmake
    DESTINATION share/xyris/cmake
)
install(PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/xyris-build/xyris-build.py
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/xyris-build/xyris-package.py
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/xyris-build/xyris-dev.py
    DESTINATION bin
)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/rust/
    DESTINATION share/xyris/rust/xyris-sdk
    PATTERN "target" EXCLUDE
)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../toolchain/xyris-rust.toml
    DESTINATION share/xyris/rust
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/XyrisSDKConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/XyrisSDKConfig.cmake
    INSTALL_DESTINATION lib/cmake/XyrisSDK
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/XyrisSDKConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY ExactVersion
)

install(EXPORT XyrisSDKTargets
    FILE XyrisSDKTargets.cmake
    NAMESPACE Xyris::
    DESTINATION lib/cmake/XyrisSDK
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/XyrisSDKConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/XyrisSDKConfigVersion.cmake
    DESTINATION lib/cmake/XyrisSDK
)
