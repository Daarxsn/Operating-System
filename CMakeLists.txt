cmake_minimum_required(VERSION 3.20)

project(XyrisOS C ASM)

include(CTest)

# =====================================================
# Kernel
# =====================================================

add_executable(kernel.elf

    # Core
    kernel/kernel.c
    kernel/main.c
    kernel/init.c

    # Memory
    kernel/memory/paging.c
    kernel/memory/heap.c
    kernel/memory/frame.c
    kernel/memory/vmm.c
    kernel/memory/pmm.c

    # Graphics / UI
    kernel/graphics/framebuffer.c
    kernel/graphics/font.c
    kernel/graphics/terminal.c
    kernel/ui/ui.c
    kernel/ui/window.c

    # Objects / UKOM
    kernel/object/object.c
    kernel/object/ukom.c
    kernel/object/process.c
    kernel/object/thread.c
    kernel/object/driver.c
    kernel/object/device.c
    kernel/object/timer.c
    kernel/object/event.c
    kernel/object/resource.c

    # Syscall / ABI / SDK
    kernel/syscall/syscall.c
    kernel/syscall/syscall_table.c
    kernel/syscall/sdk_services.c

    # Loader / Process
    kernel/loader/elf.c
    kernel/process/user.c


    # CPU
    kernel/cpu/gdt.c
    kernel/cpu/idt.c
    kernel/cpu/isr.c
    kernel/cpu/exceptions.c
    kernel/cpu/page_fault.c
    kernel/cpu/interrupt.S
    kernel/cpu/io.c
    kernel/cpu/pic.c
    kernel/cpu/irq.c
    kernel/cpu/pit.c
    kernel/cpu/cpu.c
    kernel/cpu/lapic.c
    kernel/cpu/ioapic.c


    # Debug
    kernel/debug/panic.c
    kernel/debug/print.c
    kernel/debug/hex.c


    # Boot
    kernel/boot/boot.c
    kernel/boot/splash.c

    # Drivers
    kernel/drivers/driver.c
    kernel/drivers/keyboard.c
    kernel/drivers/mouse.c
    kernel/drivers/serial.c
    kernel/drivers/pci.c

    ## Process 

    kernel/process/process.c
    kernel/process/thread.c
    kernel/process/queue.c
    kernel/process/scheduler.c
    kernel/process/context.c
    kernel/process/switch.S
    kernel/process/idle.c
    kernel/process/user_thread.S

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

add_subdirectory(simulator)
add_subdirectory(xyris-sdk)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(BUILD_TESTING)
    add_test(
        NAME xyris-xapp-packaging-test
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/xapp/test_xapp.py
    )
endif()
