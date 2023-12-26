# cmake version
cmake_minimum_required(VERSION 3.16.3)
# project name
project(server)

# set c++ version
set(CMAKE_CXX_STANDARD 17)

# add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer)
# link_libraries(-fsanitize=address -fsanitize=leak -fsanitize=undefined)

# include dir add,split by<space>
include_directories(../util)

# link dir add
# link_directories()

# add source
aux_source_directory(./game DIR_SRCS)
aux_source_directory(./net DIR_SRCS)
aux_source_directory(../util DIR_SRCS)

# 设定生成的可执行二进制文件存放的存放目录
set(EXECUTABLE_OUTPUT_PATH ../../bin)

add_executable(server ./main.cpp ${DIR_SRCS})
add_executable(term ./term_main.cpp ${DIR_SRCS})

# link lib
find_package(fmt)
target_link_libraries(server fmt::fmt)
target_link_libraries(term fmt::fmt)
