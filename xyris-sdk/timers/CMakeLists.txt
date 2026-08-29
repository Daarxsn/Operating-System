cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKTimers C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-timers INTERFACE)

target_include_directories(xyris-sdk-timers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)

target_link_libraries(xyris-sdk-timers INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-timers-c-header-test tests/timers_header_test.c)
target_link_libraries(xyris-sdk-timers-c-header-test PRIVATE xyris-sdk-timers)

add_executable(xyris-sdk-timers-cpp-header-test tests/timers_header_test.cpp)
target_link_libraries(xyris-sdk-timers-cpp-header-test PRIVATE xyris-sdk-timers)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-timers-c-header-test COMMAND xyris-sdk-timers-c-header-test)
    add_test(NAME xyris-sdk-timers-cpp-header-test COMMAND xyris-sdk-timers-cpp-header-test)
endif()

install(DIRECTORY include/
    DESTINATION include
)
