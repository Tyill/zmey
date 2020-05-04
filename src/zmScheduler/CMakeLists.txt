cmake_minimum_required(VERSION 3.0.2)

project(zmScheduler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Lib/)

FILE(GLOB_RECURSE SRC_zmSch "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

find_package(PostgreSQL REQUIRED)

add_executable(zmScheduler ${SRC_zmSch})

target_include_directories(zmScheduler PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(zmScheduler PRIVATE ${PostgreSQL_LIBRARIES})
