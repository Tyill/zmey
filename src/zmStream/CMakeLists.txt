cmake_minimum_required(VERSION 3.0.2)

project(zmStream)

if(WIN32)
    add_definitions(-DZMSTREAM_DLL_EXPORTS)
endif(WIN32)

FILE(GLOB_RECURSE SRC_Cl "*.cpp" "*.h")

add_library(zmStream SHARED ${SRC_Cl})
