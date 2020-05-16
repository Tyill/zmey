cmake_minimum_required(VERSION 3.0.2)

project(zmScheduler)

FILE(GLOB_RECURSE SRC_zmSch "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmScheduler ${SRC_zmSch})

target_link_libraries(zmScheduler PRIVATE zmCommon zmDbProvider)
