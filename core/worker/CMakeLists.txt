cmake_minimum_required(VERSION 3.0.2)

project(zmworker)

FILE(GLOB_RECURSE SRC_zmWkr "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmworker ${SRC_zmWkr})

target_link_libraries(zmworker PRIVATE common base)