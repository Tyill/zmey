cmake_minimum_required(VERSION 3.0.2)

project(zmManager)

FILE(GLOB_RECURSE SRC_zmMr "*.cpp" "*.h" ".rc")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmManager ${SRC_zmMr})

#target_link_libraries (zmManager sqlite3)