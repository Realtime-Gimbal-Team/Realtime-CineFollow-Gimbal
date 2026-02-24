cmake_minimum_required(VERSION 3.10)
project(RealtimeCineFollowGimbal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# $ENV{HOME} 会自动解析为你的用户主目录 (即 /home/jack/)
set(ncnn_DIR "$ENV{HOME}/ncnn/build/install/lib/cmake/ncnn")

# 寻找外部依赖库
find_package(OpenCV REQUIRED)
find_package(ncnn REQUIRED) # [新增] 寻找刚才编译好的 NCNN

# --- 【核心修复：强行注入 NCNN 的绝对物理头文件路径】 ---
include_directories(include ${OpenCV_INCLUDE_DIRS} "$ENV{HOME}/ncnn/build/install/include")

# 收集源码
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 生成可执行文件
add_executable(CineFollow apps/main.cpp ${SOURCES})

# 链接核心库 (将 ncnn 缝合进你的程序)
target_link_libraries(CineFollow ${OpenCV_LIBS} ncnn)