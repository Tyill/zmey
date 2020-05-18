cmake_minimum_required(VERSION 3.0.2)

project(zmManager)

FILE(GLOB_RECURSE SRC_zmMnr "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_library(zmManager ${SRC_zmMnr})

target_link_libraries(zmManager PRIVATE zmCommon)