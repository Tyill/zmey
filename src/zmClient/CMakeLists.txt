cmake_minimum_required(VERSION 3.0.2)

project(zmClient)

FILE(GLOB_RECURSE SRC_zmCln "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_library(zmClient SHARED ${SRC_zmCln})

target_link_libraries(zmClient PRIVATE zmCommon zmDbProvider)