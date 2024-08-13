cmake_minimum_required(VERSION 3.12)
project(Graph)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(main ${SOURCES})

# Set compiler flags
target_compile_options(main PRIVATE -O2 -Werror -g)