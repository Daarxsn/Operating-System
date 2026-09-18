cmake_minimum_required(VERSION 3.20)

add_library(xyris-userspace-system-service STATIC
    system_service.c
)

target_link_libraries(xyris-userspace-system-service PUBLIC Xyris::SDK)

target_include_directories(xyris-userspace-system-service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(xyris-userspace-system-service PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fno-stack-check
    -fno-pic
    -fno-pie
    -fno-lto
    -ffunction-sections
    -fdata-sections
    -m64
    -march=x86-64
    -mabi=sysv
    -mno-80387
    -mno-mmx
    -mno-sse
    -mno-sse2
    -mno-red-zone
    -Wall
    -Wextra
    -Werror
)

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(xyris-system-service-header-test
        system_service_header_test.c
    )
    target_include_directories(xyris-system-service-header-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(xyris-system-service-header-test PRIVATE Xyris::SDK)
    target_compile_options(xyris-system-service-header-test PRIVATE
        -Wall -Wextra -Werror
    )
    add_test(NAME xyris-system-service-header-test
        COMMAND xyris-system-service-header-test
    )
endif()
