# XyrisOS Toolchain Configuration

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_ASM_COMPILER gcc)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_FLAGS "-ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pic -fno-pie -ffunction-sections -fdata-sections -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Wall -Wextra -Werror")

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -static")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)