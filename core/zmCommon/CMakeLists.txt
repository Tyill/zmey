cmake_minimum_required(VERSION 3.0.2)

project(zmCommon)

FILE(GLOB_RECURSE SRC_zmCmn "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

add_library(zmCommon STATIC ${SRC_zmCmn})