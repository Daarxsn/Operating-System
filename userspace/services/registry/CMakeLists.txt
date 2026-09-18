cmake_minimum_required(VERSION 3.20)

add_library(xyris-userspace-service-registry STATIC
    service_registry.c
)

target_link_libraries(xyris-userspace-service-registry PUBLIC Xyris::SDK)

target_include_directories(xyris-userspace-service-registry PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(xyris-userspace-service-registry PRIVATE
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

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(xyris-service-registry-test
        service_registry_test.c
        service_registry.c
    )
    target_include_directories(xyris-service-registry-test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/xyris-sdk/core/include
        ${CMAKE_SOURCE_DIR}/abi/include
    )
    target_compile_options(xyris-service-registry-test PRIVATE
        -Wall -Wextra -Werror
    )
    add_test(NAME xyris-service-registry-test
        COMMAND xyris-service-registry-test
    )

    add_executable(xyris-service-protocol-test
        service_protocol_test.c
    )
    target_include_directories(xyris-service-protocol-test PRIVATE
        ${CMAKE_SOURCE_DIR}/abi/include
    )
    target_compile_options(xyris-service-protocol-test PRIVATE
        -Wall -Wextra -Werror
    )
    add_test(NAME xyris-service-protocol-test
        COMMAND xyris-service-protocol-test
    )
endif()
