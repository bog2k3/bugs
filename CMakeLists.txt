# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name

project(bugs)
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/dist/lib)

# The following folder will be included
include_directories("${BOGLFW_DIST_DIR}/include")
link_directories("${BOGLFW_DIST_DIR}/lib")

# add sources
file(GLOB_RECURSE sources	bugs/*.h)
file(GLOB_RECURSE sources 	bugs/*.cpp)

add_executable(bugs ${sources})

#add libraries
target_link_libraries(bugs PUBLIC boglfw GLEW glfw GL rt png Box2D dl pthread)

set_property(TARGET bugs PROPERTY CXX_STANDARD 14)
target_compile_options(bugs PUBLIC -Wall -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")


