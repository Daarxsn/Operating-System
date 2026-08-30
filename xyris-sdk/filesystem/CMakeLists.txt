cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKFilesystem C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-filesystem INTERFACE)

target_include_directories(xyris-sdk-filesystem
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)

target_link_libraries(xyris-sdk-filesystem INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-filesystem-c-header-test tests/filesystem_header_test.c)
target_link_libraries(xyris-sdk-filesystem-c-header-test PRIVATE xyris-sdk-filesystem)

add_executable(xyris-sdk-filesystem-cpp-header-test tests/filesystem_header_test.cpp)
target_link_libraries(xyris-sdk-filesystem-cpp-header-test PRIVATE xyris-sdk-filesystem)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-filesystem-c-header-test COMMAND xyris-sdk-filesystem-c-header-test)
    add_test(NAME xyris-sdk-filesystem-cpp-header-test COMMAND xyris-sdk-filesystem-cpp-header-test)
endif()

install(DIRECTORY include/
    DESTINATION include
)
