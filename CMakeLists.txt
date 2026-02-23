cmake_minimum_required(VERSION 3.10)
project(RealtimeCineFollowGimbal)

# 设置 C++ 17 标准 (工业级主流)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动寻找 OpenCV
# find_package(OpenCV REQUIRED)

# 包含 include 目录，让编译器能找到你的“菜单”
include_directories(include)

# 递归寻找 src 目录下所有的 .cpp 文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 生成可执行文件
add_executable(CineFollow apps/main.cpp ${SOURCES})

# 链接 OpenCV 库
# target_link_libraries(CineFollow ${OpenCV_LIBS})
