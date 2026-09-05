cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKMemory C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-memory INTERFACE)

target_include_directories(xyris-sdk-memory
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../core/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(xyris-sdk-memory INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-memory-c-header-test tests/memory_header_test.c)
target_link_libraries(xyris-sdk-memory-c-header-test PRIVATE xyris-sdk-memory)

add_executable(xyris-sdk-memory-cpp-header-test tests/memory_header_test.cpp)
target_link_libraries(xyris-sdk-memory-cpp-header-test PRIVATE xyris-sdk-memory)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-memory-c-header-test COMMAND xyris-sdk-memory-c-header-test)
    add_test(NAME xyris-sdk-memory-cpp-header-test COMMAND xyris-sdk-memory-cpp-header-test)
endif()

install(DIRECTORY include/
    DESTINATION include
)
