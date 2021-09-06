cmake_minimum_required(VERSION 3.0.2)

project(common)

FILE(GLOB_RECURSE SRC_zmCmn "*.cpp" "*.h")

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

if(WIN32)         
  add_definitions(-DASIO_STANDALONE)  
  add_definitions(-D_WIN32_WINDOWS)
endif()

add_library(common STATIC ${SRC_zmCmn})