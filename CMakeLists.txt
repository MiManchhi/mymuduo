cmake_minimum_required(VERSION 2.5)
project(mymuduo)

# 设置动态库生成路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 以及 C++标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")
# 定义参加编译的源代码文件
aux_source_directory(. SRC_LISTS)
# 编译生成动态库
add_library(mymuduo SHARED ${SRC_LISTS})