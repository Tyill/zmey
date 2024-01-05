cmake_minimum_required(VERSION 3.0.2)

project(zmclient)

FILE(GLOB_RECURSE SRC_zmCln "*.cpp" "*.h")

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)
  
if(WIN32)
  add_definitions(-DZMEYDLL_EXPORTS) 
endif(WIN32)

add_library(zmclient SHARED ${SRC_zmCln})

target_link_libraries(zmclient PRIVATE common base db_provider)