cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

# 项目名称
project(pico_foc_project C CXX ASM)

# 初始化 SDK
pico_sdk_init()

# ==========================================
# 1. 源码搜集与过滤 (SimpleFOC 核心)
# ==========================================
file(GLOB_RECURSE SIMPLEFOC_SOURCES "SimpleFOC_Core/*.cpp")
list(FILTER SIMPLEFOC_SOURCES EXCLUDE REGEX "hardware_specific")

# ==========================================
# 2. 定义可执行文件及编译清单
# ==========================================
add_executable(pico_foc
    src/main.cpp
    src/Motor.cpp
    src/UART_Parser.cpp
    src/PicoDriver3PWM.cpp
    ${SIMPLEFOC_SOURCES}
)

# ==========================================
# 3. 编译选项：注入 Arduino 兼容层补丁
# 注意：只在编译 C++ 时注入，保护纯 C 文件免受语法污染
# ==========================================
target_compile_options(pico_foc PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-include "${CMAKE_CURRENT_SOURCE_DIR}/include/Arduino.h">)

# ==========================================
# 4. 头文件搜索路径 (Include Directories)
# ==========================================
target_include_directories(pico_foc PRIVATE 
    include 
    SimpleFOC_Core
    SimpleFOC_Core/common
    SimpleFOC_Core/common/base_classes
    SimpleFOC_Core/drivers
    SimpleFOC_Core/sensors
    SimpleFOC_Core/communication
)

# ==========================================
# 5. 【核心枢纽】链接底层硬件库
# ==========================================
target_link_libraries(pico_foc 
    pico_stdlib
    hardware_pwm      # SimpleFOC 电机驱动必须
    hardware_adc      # 电流采样/传感器必须
    hardware_gpio     # 基础引脚控制必须
    hardware_spi      # 磁编码器通信必须
    hardware_i2c      # I2C 传感器必须
    hardware_sync     # 多核/中断同步保护必须
    hardware_uart     # <--- 【关键修复】物理串口解析器 (UART_Parser) 必须！
)

# ==========================================
# 6. 固件输出与调试接口配置
# ==========================================
# 开启 USB CDC 虚拟串口打印，关闭默认的 UART0 打印（防止与外接树莓派的信号互相污染）
pico_enable_stdio_usb(pico_foc 1)
pico_enable_stdio_uart(pico_foc 0)

# 生成最终可烧录的 .uf2 固件
pico_add_extra_outputs(pico_foc)