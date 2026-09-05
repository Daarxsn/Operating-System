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
target_include_directories(xyris-sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
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
endif()

install(FILES core/include/xyris/sdk.h DESTINATION include/xyris)
