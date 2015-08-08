# This is an example project to show and test the usage of the FindOpenCL
# script.

cmake_minimum_required( VERSION 3.2 )
project(MonaLearning)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)

include_directories( ${OPENCL_INCLUDE_DIR} )
include_directories( ${OPENGL_INCLUDE_DIR} )
include_directories( ${GLEW_INCLUDE_DIR} )
include_directories( ${GLFW_INCLUDE_DIR} )
#include_directories( "${CMAKE_SOURCE_DIR}/khronos" )




SUBDIRS(src)

