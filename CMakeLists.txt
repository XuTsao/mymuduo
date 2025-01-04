cmake_minimum_required(VERSION 2.5)
project(mymuduo)

# mymuduo最终编译成so动态库，设置动态库路径，放在更目录lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 指定编译选项 以及启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义参与编译的源代码文件 把当前根目录下的源文件名字组合起来放在SRC_LIST变量中
aux_source_directory(. SRC_LIST)
# 编译动态库mymuduo SHARED动态的 ${SRC_LIST}表示引用这个变量
add_library(mymuduo SHARED ${SRC_LIST})