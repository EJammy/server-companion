cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(server_companion_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_library(sha1
  tiny-HMAC-c/src/sha1.c
  tiny-HMAC-c/src/sha1.h
)
add_library(hmac
  tiny-HMAC-c/src/hmac.c
  tiny-HMAC-c/src/hmac.h
)
target_link_libraries(hmac sha1)
add_executable(main
  main.c
)
target_link_libraries(main hmac)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)
pico_add_extra_outputs(main)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
target_link_libraries(main pico_cyw43_arch_lwip_threadsafe_background pico_stdlib)
