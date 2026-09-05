cmake_minimum_required(VERSION 3.20)
include(CTest)
project(XyrisSDKNetworking C CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-networking INTERFACE)
target_include_directories(xyris-sdk-networking INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../core/include>
        $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(xyris-sdk-networking INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-networking-c-header-test tests/networking_header_test.c)
target_link_libraries(xyris-sdk-networking-c-header-test PRIVATE xyris-sdk-networking)
add_executable(xyris-sdk-networking-cpp-header-test tests/networking_header_test.cpp)
target_link_libraries(xyris-sdk-networking-cpp-header-test PRIVATE xyris-sdk-networking)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-networking-c-header-test COMMAND xyris-sdk-networking-c-header-test)
    add_test(NAME xyris-sdk-networking-cpp-header-test COMMAND xyris-sdk-networking-cpp-header-test)
endif()

install(DIRECTORY include/ DESTINATION include)
