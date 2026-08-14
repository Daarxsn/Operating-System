# XyrisOS x86_64 freestanding toolchain
#
# Prefer a true x86_64-elf cross compiler when installed.  The host GCC
# fallback is accepted only when it targets x86_64, because the kernel is
# already built with fully freestanding x86_64 flags.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

find_program(XYRIS_X86_64_ELF_GCC x86_64-elf-gcc)
find_program(XYRIS_HOST_GCC gcc)

if(XYRIS_X86_64_ELF_GCC)
    set(XYRIS_C_COMPILER "${XYRIS_X86_64_ELF_GCC}")
else()
    if(NOT XYRIS_HOST_GCC)
        message(FATAL_ERROR "XyrisOS requires x86_64-elf-gcc or gcc")
    endif()
    execute_process(
        COMMAND "${XYRIS_HOST_GCC}" -dumpmachine
        OUTPUT_VARIABLE XYRIS_GCC_TARGET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE XYRIS_GCC_RESULT
    )
    if(NOT XYRIS_GCC_RESULT EQUAL 0 OR NOT XYRIS_GCC_TARGET MATCHES "x86_64")
        message(FATAL_ERROR "Host GCC must target x86_64; detected '${XYRIS_GCC_TARGET}'")
    endif()
    set(XYRIS_C_COMPILER "${XYRIS_HOST_GCC}")
endif()

set(CMAKE_C_COMPILER "${XYRIS_C_COMPILER}")
set(CMAKE_ASM_COMPILER "${XYRIS_C_COMPILER}")
set(CMAKE_C_STANDARD 23)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS_INIT "-ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pic -fno-pie -ffunction-sections -fdata-sections -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Wall -Wextra -Werror")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib -static")
