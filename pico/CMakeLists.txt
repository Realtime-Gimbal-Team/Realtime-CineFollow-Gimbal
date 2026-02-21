cmake_minimum_required(VERSION 3.13)
# --- 把下面这行加在 include(pico_sdk_import.cmake) 的前面 ---
set(PICO_SDK_PATH "C:/Users/18249/.pico-sdk/sdk/2.2.0")

# 导入神级 SDK 钥匙
include(pico_sdk_import.cmake)

# 项目命名
project(pico_foc_project C CXX ASM)

# 点火启动 SDK
pico_sdk_init()

# 1. 严格指定这三个干活的 cpp 文件在哪里！
add_executable(pico_foc
    src/main.cpp
    src/Motor.cpp
    src/UART_Parser.cpp
)

# 2. 绝对结界：只允许在 include 文件夹里找图纸
target_include_directories(pico_foc PRIVATE include)

# 3. 链接标准库
target_link_libraries(pico_foc pico_stdlib)

# 4. 防干扰打印设置
pico_enable_stdio_usb(pico_foc 1)
pico_enable_stdio_uart(pico_foc 0)

# 5. 生成 uf2 固件
pico_add_extra_outputs(pico_foc)