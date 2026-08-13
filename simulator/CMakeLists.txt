cmake_minimum_required(VERSION 3.20)

project(XyrisSim C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

# -------------------------------------------------
# XyrisSim Executable
# -------------------------------------------------

add_executable(XyrisSim
    src/main.c
    src/simulator.c
    src/ukom_sim.c
    src/cpu.c
    src/memory.c
    src/display.c
    src/system.c
)

# -------------------------------------------------
# Separate Test Executables
# -------------------------------------------------

add_executable(test_system
    ../tests/test_system_main.c
    ../tests/test_system.c
    src/system.c
    src/cpu.c
    src/memory.c
    src/display.c
)

add_executable(test_memory
    ../tests/test_memory_main.c
    ../tests/test_memory.c
    src/memory.c
)

add_executable(test_cpu
    ../tests/test_cpu_main.c
    ../tests/test_cpu.c
    src/cpu.c
)

add_executable(test_simulator
    ../tests/test_simulator_main.c
    ../tests/test_simulator.c
    src/simulator.c
    src/ukom_sim.c
    src/system.c
    src/cpu.c
    src/memory.c
    src/display.c
)

# -------------------------------------------------
# Kernel UKOM Tests
# -------------------------------------------------

add_executable(test_ukom
    ../tests/test_ukom.c
    ../kernel/foundation2/ukom.c
)

target_include_directories(test_ukom PRIVATE
    ../kernel/include
)

# -------------------------------------------------
# Test Include Directories
# -------------------------------------------------

target_include_directories(test_system PRIVATE include)
target_include_directories(test_memory PRIVATE include)
target_include_directories(test_cpu PRIVATE include)
target_include_directories(test_simulator PRIVATE include)

# -------------------------------------------------
# Testing
# -------------------------------------------------

enable_testing()

add_test(
    NAME XyrisOS_System_Tests
    COMMAND test_system
)

add_test(
    NAME XyrisOS_Memory_Tests
    COMMAND test_memory
)

add_test(
    NAME XyrisOS_CPU_Tests
    COMMAND test_cpu
)

add_test(
    NAME XyrisOS_Simulator_Tests
    COMMAND test_simulator
)

add_test(
    NAME XyrisOS_UKOM_Tests
    COMMAND test_ukom
)