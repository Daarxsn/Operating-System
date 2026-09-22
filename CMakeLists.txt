cmake_minimum_required(VERSION 3.20)

project(XyrisOS C ASM)

include(CTest)

option(XYRIS_BUILD_SIMULATOR "Build host-side simulator targets from the top-level project" OFF)

# =====================================================
# Kernel
# =====================================================

set(KERNEL_SOURCES
    kernel/core/kernel.c
    kernel/cpu/cpu.c
    kernel/cpu/exceptions.c
    kernel/cpu/gdt.c
    kernel/cpu/idt.c
    kernel/cpu/interrupt.S
    kernel/cpu/io.c
    kernel/cpu/ioapic.c
    kernel/cpu/irq.c
    kernel/cpu/isr.c
    kernel/cpu/lapic.c
    kernel/cpu/page_fault.c
    kernel/cpu/pic.c
    kernel/cpu/pit.c
    kernel/debug/hex.c
    kernel/debug/panic.c
    kernel/debug/print.c
    kernel/drivers/driver.c
    kernel/drivers/keyboard.c
    kernel/drivers/mouse.c
    kernel/drivers/pci.c
    kernel/drivers/serial.c
    kernel/foundation2/capability.c
    kernel/foundation2/config.c
    kernel/foundation2/event.c
    kernel/foundation2/resource.c
    kernel/foundation2/time.c
    kernel/foundation2/ukom.c
    kernel/fs/file.c
    kernel/fs/ramfs.c
    kernel/fs/vfs.c
    kernel/graphics/font.c
    kernel/graphics/framebuffer.c
    kernel/graphics/graphics.c
    kernel/graphics/primitives.c
    kernel/image/image.c
    kernel/image/logo.c
    kernel/init/init.c
    kernel/lib/string.c
    kernel/loader/elf.c
    kernel/logger/logger.c
    kernel/hardware/hardware_inventory.c
    kernel/hardware/pci_inventory.c
    kernel/drivers/block.c
    kernel/drivers/ahci.c
    kernel/drivers/xhci.c
    kernel/memory/bitmap.c
    kernel/memory/heap.c
    kernel/memory/hhdm.c
    kernel/memory/memory_map.c
    kernel/memory/pmm.c
    kernel/memory/vmm.c
    kernel/process/context.c
    kernel/process/idle.c
    kernel/process/process.c
    kernel/process/queue.c
    kernel/process/scheduler.c
    kernel/process/switch.S
    kernel/process/thread.c
    kernel/process/user.c
    kernel/process/user_thread.S
    kernel/syscall/sdk_services.c
    kernel/syscall/syscall.c
    kernel/syscall/syscall_isr.c
    kernel/terminal/terminal.c
    kernel/tests/foundation_tests.c
    kernel/tests/fs_tests.c
    kernel/tests/memory_tests.c
    kernel/tests/scheduler_test.c
    kernel/tests/sdk_services_tests.c
    kernel/tests/tests.c
    kernel/ui/ui.c
    kernel/boot/boot.c
    kernel/boot/splash.c
)

add_executable(kernel.elf
    ${KERNEL_SOURCES}
)


target_include_directories(kernel.elf PRIVATE
    kernel
    kernel/include
)


target_compile_options(kernel.elf PRIVATE

    -ffreestanding

    -fno-stack-protector
    -fno-stack-check
    -fno-lto

    -fno-pic
    -fno-pie

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

    -mcmodel=kernel


    -Wall
    -Wextra
    -Werror
)



target_link_options(kernel.elf PRIVATE

    -nostdlib
    -static

    -Wl,-m,elf_x86_64

    -Wl,-z,max-page-size=0x1000

    -Wl,--gc-sections

    -T ${CMAKE_SOURCE_DIR}/boot/linker.ld
)

# =====================================================
# Ring-3 User Test Application
# =====================================================

add_executable(user_test.elf
    tests/user_app.S
)

set_target_properties(user_test.elf PROPERTIES
    OUTPUT_NAME "test.elf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/user"
)

target_link_options(user_test.elf PRIVATE
    -nostdlib
    -static
    -Wl,-m,elf_x86_64
    -Wl,-z,max-page-size=0x1000
    -Wl,--gc-sections
    -Wl,-T,${CMAKE_SOURCE_DIR}/tests/user.ld
)

add_custom_command(
    TARGET user_test.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -I elf64-x86-64
        -O binary
        $<TARGET_FILE:user_test.elf>
        ${CMAKE_BINARY_DIR}/user/test.bin
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/user/user_test_blob.o
    COMMAND ${CMAKE_COMMAND} -E chdir
        ${CMAKE_BINARY_DIR}/user
        ${CMAKE_OBJCOPY}
        -I binary
        -O elf64-x86-64
        -B i386:x86-64
        test.elf
        user_test_blob.o
    DEPENDS user_test.elf
    VERBATIM
)

add_custom_target(user_test_blob
    DEPENDS ${CMAKE_BINARY_DIR}/user/user_test_blob.o
)

# =====================================================
# Embed User Test ELF into Kernel
# =====================================================

set(USER_TEST_BLOB ${CMAKE_BINARY_DIR}/user/user_test_blob.o)

target_sources(kernel.elf PRIVATE
    ${USER_TEST_BLOB}
)

# =====================================================
# Tests
# =====================================================

if(XYRIS_BUILD_SIMULATOR)
    add_subdirectory(simulator)
endif()
add_subdirectory(xyris-sdk)
add_subdirectory(userspace)

# Member 9 userspace bootstrap image.
set(USER_INIT_BLOB ${CMAKE_BINARY_DIR}/user/userspace_init_blob.o)
add_dependencies(kernel.elf userspace_init_blob)
target_sources(kernel.elf PRIVATE ${USER_INIT_BLOB})

find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(BUILD_TESTING)
    add_test(
        NAME xyris-xapp-packaging-test
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/xapp/test_xapp.py
    )
endif()
