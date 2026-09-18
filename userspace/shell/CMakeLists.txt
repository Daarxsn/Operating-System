cmake_minimum_required(VERSION 3.20)

# ============================================================
# XyrisOS Userspace Shell
# Member 9: command parsing and command execution core.
# Terminal input/output is intentionally injected through a callback.
# ============================================================

add_library(xyris-userspace-shell STATIC
    shell.c
)

target_include_directories(xyris-userspace-shell PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/userspace/common
)

target_link_libraries(xyris-userspace-shell PUBLIC
    xyris-userspace-service-registry
    xyris-userspace-system-service
    xyris-userspace-utilities
)

target_compile_options(xyris-userspace-shell PRIVATE
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

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
    add_executable(xyris-shell-test
        tests/shell_test.c
    )

    target_include_directories(xyris-shell-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/userspace/common
        ${CMAKE_SOURCE_DIR}/userspace/utilities
    ${CMAKE_SOURCE_DIR}/userspace/common
        ${CMAKE_SOURCE_DIR}/userspace/init
        ${CMAKE_SOURCE_DIR}/userspace/services/registry
        ${CMAKE_SOURCE_DIR}/userspace/services/system
    )

    target_link_libraries(xyris-shell-test PRIVATE
        xyris-userspace-shell
        xyris-userspace-service-registry
    )

    target_compile_options(xyris-shell-test PRIVATE
        -Wall
        -Wextra
        -Werror
    )
    target_link_options(xyris-shell-test PRIVATE -no-pie)

    add_test(NAME xyris-shell-test COMMAND xyris-shell-test)
endif()
