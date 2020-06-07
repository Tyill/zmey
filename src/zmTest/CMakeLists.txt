cmake_minimum_required(VERSION 3.0.2)

project(zmTest)

FILE(GLOB_RECURSE SRC_zmTest "*.cpp" "*.h")

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmTest ${SRC_zmTest})

target_link_libraries(zmTest PRIVATE zmCommon zmDbProvider)
