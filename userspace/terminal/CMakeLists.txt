cmake_minimum_required(VERSION 3.20)

# ============================================================
# XyrisOS Userspace Terminal Adapter
# Member 9: interactive shell loop and line editing.
# Hardware/driver access is intentionally injected by callbacks.
# ============================================================

add_library(xyris-userspace-terminal STATIC
    terminal.c
)

target_include_directories(xyris-userspace-terminal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/userspace/common
    ${CMAKE_SOURCE_DIR}/userspace/shell
)

target_link_libraries(xyris-userspace-terminal PUBLIC
    xyris-userspace-shell
)

target_compile_options(xyris-userspace-terminal PRIVATE
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
    add_executable(xyris-terminal-test
        tests/terminal_test.c
    )

    target_include_directories(xyris-terminal-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/userspace/common
        ${CMAKE_SOURCE_DIR}/userspace/shell
        ${CMAKE_SOURCE_DIR}/userspace/utilities
        ${CMAKE_SOURCE_DIR}/userspace/services/registry
        ${CMAKE_SOURCE_DIR}/userspace/services/system
    )

    target_link_libraries(xyris-terminal-test PRIVATE
        xyris-userspace-terminal
        xyris-userspace-shell
        xyris-userspace-utilities
        xyris-userspace-system-service
        xyris-userspace-service-registry
    )

    target_compile_options(xyris-terminal-test PRIVATE
        -Wall
        -Wextra
        -Werror
    )
    target_link_options(xyris-terminal-test PRIVATE -no-pie)

    add_test(NAME xyris-terminal-test COMMAND xyris-terminal-test)
endif()
