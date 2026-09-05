cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKEvents C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-events INTERFACE)

target_include_directories(xyris-sdk-events
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../core/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(xyris-sdk-events INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-events-c-header-test tests/events_header_test.c)
target_link_libraries(xyris-sdk-events-c-header-test PRIVATE xyris-sdk-events)

add_executable(xyris-sdk-events-cpp-header-test tests/events_header_test.cpp)
target_link_libraries(xyris-sdk-events-cpp-header-test PRIVATE xyris-sdk-events)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-events-c-header-test COMMAND xyris-sdk-events-c-header-test)
    add_test(NAME xyris-sdk-events-cpp-header-test COMMAND xyris-sdk-events-cpp-header-test)
endif()

install(DIRECTORY include/
    DESTINATION include
)
