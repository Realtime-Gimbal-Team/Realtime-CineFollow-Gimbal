cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350)

# 1. SDK 与工具链路径
set(PICO_SDK_PATH "C:/Users/18249/.pico-sdk/sdk/2.2.0") 
set(PICO_TOOLCHAIN_PATH "C:/Users/18249/.pico-sdk/toolchain/14_2_Rel1")

include(pico_sdk_import.cmake)
project(pico_foc_project C CXX ASM)
pico_sdk_init()

# 2. 搜集 SimpleFOC 源码并严格过滤无用驱动
file(GLOB_RECURSE SIMPLEFOC_SOURCES "SimpleFOC_Core/*.cpp" "SimpleFOC_Core/*.c")
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "hardware_specific")
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "Stepper") 
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "BLDCDriver6PWM")

# 3. 声明执行文件及业务代码 (已剔除废弃的传感器文件)
add_executable(pico_foc 
    src/main.cpp 
    src/Motor.cpp 
    src/PicoDriver3PWM.cpp
    src/UART_Parser.cpp
)

# 将核心算法绑定到执行文件
target_sources(pico_foc PRIVATE ${SIMPLEFOC_SOURCES})

# 4. 注入 Arduino 补丁 (此处为高风险操作区域)
target_compile_options(pico_foc PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/include/Arduino.h>")

# 5. 头文件路径
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

# 6. 链接 Pico 底层硬件库 (已剔除无用的 SPI 和 I2C)
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

# 7. 开启虚拟串口输出
pico_enable_stdio_usb(pico_foc 1)
pico_enable_stdio_uart(pico_foc 0)
pico_add_extra_outputs(pico_foc)