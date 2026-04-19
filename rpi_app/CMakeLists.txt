cmake_minimum_required(VERSION 3.10)
project(Realtime_CineFollow_Gimbal VERSION 1.2)

# 强制使用现代 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. 寻找基础依赖库 ---
find_package(Threads REQUIRED)       
find_package(OpenCV REQUIRED)
# 🚨 唤醒 NCNN (确保你的系统路径或工程里能找到 ncnnConfig.cmake)
set(ncnn_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ncnn/lib/cmake/ncnn")
find_package(ncnn REQUIRED)     

# --- 2. 使用 PkgConfig 强抓底层的 libcamera ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

# --- 3. 指定头文件搜索路径 ---
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/vision)
include_directories(${LIBCAMERA_INCLUDE_DIRS})

# 🚨 架构师强力修复：按死 NCNN 的头文件物理路径！
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/ncnn/include")

# --- 4. 添加可执行文件 ---
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/libcam2opencv.cpp 
    src/libcam2opencv_format_converter.cpp
)

# --- 5. 链接动态/静态库 ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    ${OpenCV_LIBS}
    ${LIBCAMERA_LIBRARIES}
    turbojpeg
    ncnn  # 🚨 将 NCNN 链接进你的主程序
)

# 编译提示
message(STATUS "Project Configured for IBVS Architecture.")
message(STATUS "Strict Multi-threading, Libcamera & TurboJPEG integrated.")