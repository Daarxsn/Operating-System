cmake_minimum_required(VERSION 3.20)
include(CTest)
project(XyrisSDKSecurity C CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xyris-sdk-security INTERFACE)
target_include_directories(xyris-sdk-security INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)
target_link_libraries(xyris-sdk-security INTERFACE xyris-sdk-core)

add_executable(xyris-sdk-security-c-header-test tests/security_header_test.c)
target_link_libraries(xyris-sdk-security-c-header-test PRIVATE xyris-sdk-security)
add_executable(xyris-sdk-security-cpp-header-test tests/security_header_test.cpp)
target_link_libraries(xyris-sdk-security-cpp-header-test PRIVATE xyris-sdk-security)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-security-c-header-test COMMAND xyris-sdk-security-c-header-test)
    add_test(NAME xyris-sdk-security-cpp-header-test COMMAND xyris-sdk-security-cpp-header-test)
endif()

install(DIRECTORY include/ DESTINATION include)
