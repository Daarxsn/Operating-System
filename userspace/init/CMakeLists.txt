cmake_minimum_required(VERSION 3.20)

# System services are built separately so they can later become independent processes.
# M9.3 keeps the first service in-process because the public ABI has no spawn/exec contract yet.

# M9 userspace init and service-manager image.
# It uses the existing public Xyris SDK and does not define new ABI calls.

add_executable(xyris-userspace-init
    main.c
    service_manager.c
)

target_link_libraries(xyris-userspace-init PRIVATE
    Xyris::SDK
    xyris-userspace-system-service
    xyris-userspace-service-registry
    xyris-userspace-shell
    xyris-userspace-utilities
    xyris-userspace-terminal
)

target_compile_options(xyris-userspace-init PRIVATE
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

target_link_options(xyris-userspace-init PRIVATE
    -nostdlib
    -static
    -Wl,-m,elf_x86_64
    -Wl,-z,max-page-size=0x1000
    -Wl,--gc-sections
    -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/user.ld
)

set_target_properties(xyris-userspace-init PROPERTIES
    OUTPUT_NAME "init.elf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/user"
)

add_custom_command(
    TARGET xyris-userspace-init POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -I elf64-x86-64
        -O binary
        $<TARGET_FILE:xyris-userspace-init>
        ${CMAKE_BINARY_DIR}/user/init.bin
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/user/userspace_init_blob.o
    COMMAND ${CMAKE_COMMAND} -E chdir
        ${CMAKE_BINARY_DIR}/user
        ${CMAKE_OBJCOPY}
        -I binary
        -O elf64-x86-64
        -B i386:x86-64
        init.elf
        userspace_init_blob.o
    DEPENDS xyris-userspace-init
    VERBATIM
)

add_custom_target(userspace_init_blob
    DEPENDS ${CMAKE_BINARY_DIR}/user/userspace_init_blob.o
)


# Host-only unit test for the service lifecycle logic.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
    add_executable(xyris-service-manager-test
        tests/service_manager_test.c
        service_manager.c
    )
    target_include_directories(xyris-service-manager-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_options(xyris-service-manager-test PRIVATE
        -Wall -Wextra -Werror
    )
    add_test(NAME xyris-service-manager-test COMMAND xyris-service-manager-test)
endif()
