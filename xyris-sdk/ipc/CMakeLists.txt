cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKIPC C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-ipc INTERFACE)

target_include_directories(xyris-sdk-ipc
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../core/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(xyris-sdk-ipc INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-ipc-c-header-test tests/ipc_header_test.c)
target_link_libraries(xyris-sdk-ipc-c-header-test PRIVATE xyris-sdk-ipc)

add_executable(xyris-sdk-ipc-cpp-header-test tests/ipc_header_test.cpp)
target_link_libraries(xyris-sdk-ipc-cpp-header-test PRIVATE xyris-sdk-ipc)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-ipc-c-header-test COMMAND xyris-sdk-ipc-c-header-test)
    add_test(NAME xyris-sdk-ipc-cpp-header-test COMMAND xyris-sdk-ipc-cpp-header-test)
endif()

install(DIRECTORY include/
    DESTINATION include
)
