cmake_minimum_required(VERSION 3.0.2)

project(zmWorker)

FILE(GLOB_RECURSE SRC_zmWkr "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmWorker ${SRC_zmWkr})

target_link_libraries(zmWorker PRIVATE zmCommon)