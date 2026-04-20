cmake_minimum_required(VERSION 3.10)
project(Realtime_CineFollow_Gimbal VERSION 1.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. Locate basic dependency libraries ---
find_package(Threads REQUIRED)       
find_package(OpenCV REQUIRED)

set(ncnn_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ncnn/lib/cmake/ncnn")
find_package(ncnn REQUIRED)     

# --- 2. Use PkgConfig to force linking with the underlying libcamera ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

# --- 3. Specify header file search paths ---
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/vision)
include_directories(${LIBCAMERA_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/3rdparty/ncnn/include")

# --- 4. Add executable ---
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/libcam2opencv.cpp 
    src/libcam2opencv_format_converter.cpp
)

# --- 5. Link dynamic/static libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    ${OpenCV_LIBS}
    ${LIBCAMERA_LIBRARIES}
    turbojpeg
    ncnn  
)

# Build notes
message(STATUS "Project Configured for IBVS Architecture.")
message(STATUS "Strict Multi-threading, Libcamera & TurboJPEG integrated.")