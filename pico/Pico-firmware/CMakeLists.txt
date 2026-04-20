cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350)

# 1. SDK and toolchain paths
set(PICO_SDK_PATH "C:/Users/18249/.pico-sdk/sdk/2.2.0") 
set(PICO_TOOLCHAIN_PATH "C:/Users/18249/.pico-sdk/toolchain/14_2_Rel1")

include(pico_sdk_import.cmake)
project(pico_foc_project C CXX ASM)
pico_sdk_init()

# 2. Collect the SimpleFOC source code and strictly filter out unnecessary drivers
file(GLOB_RECURSE SIMPLEFOC_SOURCES "SimpleFOC_Core/*.cpp" "SimpleFOC_Core/*.c")
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "hardware_specific")
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "Stepper") 
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "BLDCDriver6PWM")

# 3. Declare the executable and application code 
add_executable(pico_foc
    src/main.cpp
    src/UART_Parser.cpp
    src/PicoDriver3PWM.cpp 
)

target_sources(pico_foc PRIVATE ${SIMPLEFOC_SOURCES})

# 4. Inject Arduino patches 
target_compile_options(pico_foc PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/include/Arduino.h>")

# 5.Header file path
target_include_directories(pico_foc PRIVATE 
    include 
    SimpleFOC_Core
    SimpleFOC_Core/src
    SimpleFOC_Core/common
    SimpleFOC_Core/common/base_classes
    SimpleFOC_Core/drivers
    SimpleFOC_Core/sensors
    SimpleFOC_Core/communication
)

# 6.Link the Pico low-level hardware libraries 
target_link_libraries(pico_foc 
    pico_stdlib
    hardware_pwm      
    hardware_adc      
    hardware_gpio     
    hardware_sync     
    hardware_uart     
    hardware_timer
    hardware_clocks   
)

# 7. Enable virtual serial port output
pico_enable_stdio_usb(pico_foc 1)
pico_enable_stdio_uart(pico_foc 0)
pico_add_extra_outputs(pico_foc)
