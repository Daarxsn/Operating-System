cmake_minimum_required(VERSION 3.20)
include(CTest)
project(XyrisSDKDevices C CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-devices INTERFACE)
target_include_directories(xyris-sdk-devices INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)
target_link_libraries(xyris-sdk-devices INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-devices-c-header-test tests/devices_header_test.c)
target_link_libraries(xyris-sdk-devices-c-header-test PRIVATE xyris-sdk-devices)
add_executable(xyris-sdk-devices-cpp-header-test tests/devices_header_test.cpp)
target_link_libraries(xyris-sdk-devices-cpp-header-test PRIVATE xyris-sdk-devices)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-devices-c-header-test COMMAND xyris-sdk-devices-c-header-test)
    add_test(NAME xyris-sdk-devices-cpp-header-test COMMAND xyris-sdk-devices-cpp-header-test)
endif()

install(DIRECTORY include/ DESTINATION include)
