cmake_minimum_required(VERSION 4.0)
project(VanctionProgramLanguage)

set(CMAKE_CXX_STANDARD 17)

# 设置头文件目录
include_directories(include)

# 源文件列表
set(SOURCE_FILES
    src/lexer.cpp
    src/parser.cpp
    src/main.cpp
    src/code_generator.cpp
    src/error.cpp
)

# 添加可执行文件
add_executable(vanction ${SOURCE_FILES})

# 设置输出目录
set_target_properties(vanction PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)



