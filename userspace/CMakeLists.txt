cmake_minimum_required(VERSION 3.20)

# ============================================================
# XyrisOS Userspace
# Member 9: Userspace & System Services
# ============================================================

add_subdirectory(services)
add_subdirectory(utilities)
add_subdirectory(init)

add_subdirectory(shell)

add_subdirectory(terminal)

add_subdirectory(integration)
