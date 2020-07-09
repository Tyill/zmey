cmake_minimum_required(VERSION 3.0.2)

project(zmTest)

FILE(GLOB_RECURSE SRC_zmTest "*.cpp" "*.h")

find_package(GTest REQUIRED)

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

#add_definitions(-DTASKTEST)
#add_definitions(-DAPITEST)

add_executable(zmTest ${SRC_zmTest})
 
target_link_libraries(zmTest PRIVATE zmCommon zmey zmDbProvider ${GTEST_LIBRARIES})
