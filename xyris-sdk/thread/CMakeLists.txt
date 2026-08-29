cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKThread C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-thread INTERFACE)

target_include_directories(xyris-sdk-thread
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)

target_link_libraries(xyris-sdk-thread
    INTERFACE
        xyris-sdk-core
)

add_executable(xyris-sdk-thread-c-header-test
    tests/thread_header_test.c
)
target_link_libraries(xyris-sdk-thread-c-header-test PRIVATE xyris-sdk-thread)

add_executable(xyris-sdk-thread-cpp-header-test
    tests/thread_header_test.cpp
)
target_link_libraries(xyris-sdk-thread-cpp-header-test PRIVATE xyris-sdk-thread)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-thread-c-header-test
        COMMAND xyris-sdk-thread-c-header-test
    )
    add_test(NAME xyris-sdk-thread-cpp-header-test
        COMMAND xyris-sdk-thread-cpp-header-test
    )
endif()

install(DIRECTORY include/
    DESTINATION include
)
