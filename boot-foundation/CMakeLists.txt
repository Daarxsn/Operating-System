cmake_minimum_required(VERSION 3.20)

project(BootFoundation C)

set(CMAKE_C_STANDARD 23)

add_executable(kernel.elf
    kernel/core/kernel.c
    kernel/graphics/framebuffer.c
    kernel/graphics/graphics.c
)

target_compile_options(kernel PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fno-pie
    -fno-pic
    -m64
)

add_custom_command(
    OUTPUT kernel.elf

    COMMAND ld
        -nostdlib
        -T ${CMAKE_SOURCE_DIR}/linker.ld
        -o kernel.elf
        $<TARGET_OBJECTS:kernel>

    DEPENDS kernel
)

add_custom_target(kernel_image ALL
    DEPENDS kernel.elf
)