cmake_minimum_required(VERSION 3.20)

project(XyrisSDK C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core)
add_subdirectory(process)
add_subdirectory(thread)
add_subdirectory(memory)
add_subdirectory(filesystem)
add_subdirectory(ipc)
add_subdirectory(events)
add_subdirectory(timers)
