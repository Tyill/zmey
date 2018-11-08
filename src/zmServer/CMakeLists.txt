cmake_minimum_required(VERSION 3.0.2)

project(zmServer)

if(WIN32)
    add_definitions(-DZMSRV_DLL_EXPORTS)
endif(WIN32)

FILE(GLOB_RECURSE SRC_SRV "*.cpp" "*.h")

add_library(zmServer SHARED ${SRC_SRV})
