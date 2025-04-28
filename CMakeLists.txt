cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(ohmimetro C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(ohmimetro ohmimetro.c lib/ssd1306.c)


# Configuração de I/O
pico_enable_stdio_uart(ohmimetro 0)
pico_enable_stdio_usb(ohmimetro 1)

# Bibliotecas necessárias para o projeto
target_link_libraries(ohmimetro
    pico_stdlib
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_clocks
    hardware_adc
    hardware_pwm
)

# Adiciona saídas extras (UF2, etc.)
pico_add_extra_outputs(ohmimetro)