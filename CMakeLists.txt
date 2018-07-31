# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

# Project's name

project(bugs)
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/dist/lib)

# The following folder will be included
include_directories("${BOGLFW_DIST_DIR}/include")
link_directories("${BOGLFW_DIST_DIR}/lib" "/usr/local/lib")

# add sources
file(GLOB_RECURSE sources	bugs/*.h)
file(GLOB_RECURSE sources 	bugs/*.cpp)

add_executable(bugs ${sources})

# add explicit dependency on boglfw library to force the target to be re-linked when the library is changed.
# for some reason CMake doesn't do this automatically even though we do depend on the library.
set_property(TARGET bugs PROPERTY LINK_DEPENDS "${BOGLFW_DIST_DIR}/lib/libboglfw.a")

#add libraries
target_link_libraries(bugs PUBLIC boglfw GLEW glfw3 GL rt png Box2D dl pthread X11 Xrandr Xinerama Xcursor)

set_property(TARGET bugs PROPERTY CXX_STANDARD 14)
target_compile_options(bugs PUBLIC -Wall -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")


