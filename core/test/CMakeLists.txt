cmake_minimum_required(VERSION 3.0.2)

project(zmtest)

FILE(GLOB_RECURSE SRC_zmTest "*.cpp" "*.h")

find_package(GTest REQUIRED)

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

add_executable(zmtest ${SRC_zmTest})
 
target_link_libraries(zmtest PRIVATE zmclient db_provider ${GTEST_LIBRARIES})
