cmake_minimum_required(VERSION 3.0.2)

project(zmFrame)

if(WIN32)
    add_definitions(-DZMFRAME_DLL_EXPORTS)
endif(WIN32)

FILE(GLOB_RECURSE SRC_FRAME "*.cpp" "*.h")

add_library(zmFrame SHARED ${SRC_FRAME})
