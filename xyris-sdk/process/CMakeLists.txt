cmake_minimum_required(VERSION 3.20)

include(CTest)

project(XyrisSDKProcess C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(xyris-sdk-process STATIC
    src/process.c
)

target_include_directories(xyris-sdk-process
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../abi/include
)

target_link_libraries(xyris-sdk-process PUBLIC xyris-sdk-core)

target_compile_options(xyris-sdk-process PRIVATE
    -Wall -Wextra -Werror
    -ffreestanding -fno-stack-protector -fno-stack-check
    -fno-pic -fno-pie -mno-red-zone -m64 -march=x86-64
)

add_executable(xyris-sdk-process-c-header-test tests/process_header_test.c)
target_link_libraries(xyris-sdk-process-c-header-test PRIVATE xyris-sdk-process)
target_link_options(xyris-sdk-process-c-header-test PRIVATE -no-pie)

add_executable(xyris-sdk-process-cpp-header-test tests/process_header_test.cpp)
target_link_libraries(xyris-sdk-process-cpp-header-test PRIVATE xyris-sdk-process)
target_link_options(xyris-sdk-process-cpp-header-test PRIVATE -no-pie)

if(BUILD_TESTING)
    add_test(NAME xyris-sdk-process-c-header-test COMMAND xyris-sdk-process-c-header-test)
    add_test(NAME xyris-sdk-process-cpp-header-test COMMAND xyris-sdk-process-cpp-header-test)
endif()

install(TARGETS xyris-sdk-process ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
