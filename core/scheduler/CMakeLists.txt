cmake_minimum_required(VERSION 3.0.2)

project(zmscheduler)

FILE(GLOB_RECURSE SRC_zmSch "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmscheduler ${SRC_zmSch})

target_link_libraries(zmscheduler PRIVATE common base db_provider)
