cmake_minimum_required(VERSION 3.0)
project(mprpc)

# 可执行文件和库文件生成的路径
set(CMAKE_BUILD_TYPE "Debug")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#头文件搜索路径- I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
#静态库文件搜索路径 -L
include_directories(${PROJECT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/example)

#src包含了mprpc框架所有的相关代码
add_subdirectory(src)
#example包含了mprpc框架使用的示例代码
add_subdirectory(example)