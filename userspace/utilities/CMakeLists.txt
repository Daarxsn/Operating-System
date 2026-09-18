cmake_minimum_required(VERSION 3.20)

# ============================================================
# XyrisOS Userspace Utilities
# Member 9: reusable user-level command implementations.
# ============================================================

add_library(xyris-userspace-utilities STATIC
    userspace_utilities.c
)

target_include_directories(xyris-userspace-utilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/userspace/common
)

target_link_libraries(xyris-userspace-utilities PUBLIC
    Xyris::SDK
    xyris-userspace-service-registry
    xyris-userspace-system-service
)

target_compile_options(xyris-userspace-utilities PRIVATE
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
    add_executable(xyris-userspace-utilities-test
        tests/userspace_utilities_test.c
    )
    target_include_directories(xyris-userspace-utilities-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/userspace/services/registry
        ${CMAKE_SOURCE_DIR}/userspace/services/system
        ${CMAKE_SOURCE_DIR}/userspace/common
    )
    target_link_libraries(xyris-userspace-utilities-test PRIVATE
        xyris-userspace-utilities
        xyris-userspace-service-registry
        xyris-userspace-system-service
    )
    target_compile_options(xyris-userspace-utilities-test PRIVATE
        -Wall
        -Wextra
        -Werror
        -fno-pie
    )
    target_link_options(xyris-userspace-utilities-test PRIVATE -no-pie)
    add_test(NAME xyris-userspace-utilities-test COMMAND xyris-userspace-utilities-test)
endif()
