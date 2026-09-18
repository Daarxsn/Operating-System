cmake_minimum_required(VERSION 3.20)

# ============================================================
# XyrisOS Userspace Integration
# Member 9: cross-module userspace validation.
# ============================================================

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(xyris-userspace-integration-test
        userspace_integration_test.c
        ${CMAKE_SOURCE_DIR}/userspace/services/registry/service_registry.c
        ${CMAKE_SOURCE_DIR}/userspace/terminal/terminal.c
        ${CMAKE_SOURCE_DIR}/userspace/shell/shell.c
        ${CMAKE_SOURCE_DIR}/userspace/utilities/userspace_utilities.c
    )

    target_include_directories(xyris-userspace-integration-test PRIVATE
        ${CMAKE_SOURCE_DIR}/userspace/common
        ${CMAKE_SOURCE_DIR}/userspace/init
        ${CMAKE_SOURCE_DIR}/userspace/services/registry
        ${CMAKE_SOURCE_DIR}/userspace/services/system
        ${CMAKE_SOURCE_DIR}/userspace/shell
        ${CMAKE_SOURCE_DIR}/userspace/terminal
        ${CMAKE_SOURCE_DIR}/userspace/utilities
        ${CMAKE_SOURCE_DIR}/xyris-sdk/core/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/process/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/thread/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/memory/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/filesystem/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/ipc/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/events/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/timers/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/devices/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/networking/include
        ${CMAKE_SOURCE_DIR}/xyris-sdk/security/include
        ${CMAKE_SOURCE_DIR}/abi/include
    )

    target_compile_options(xyris-userspace-integration-test PRIVATE
        -Wall
        -Wextra
        -Werror
        -fno-pie
    )
    target_link_libraries(xyris-userspace-integration-test PRIVATE
        Xyris::SDK
    )

    target_link_options(xyris-userspace-integration-test PRIVATE -no-pie)

    # The integration test validates userspace-owned modules. SDK entry points
    # used only by commands that require live Ring-3 execution are not called.
    add_test(NAME xyris-userspace-integration-test
        COMMAND xyris-userspace-integration-test)
endif()
