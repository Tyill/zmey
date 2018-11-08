cmake_minimum_required(VERSION 3.0.2)

project(zmClient)

if(WIN32)
    add_definitions(-DZMCLNT_DLL_EXPORTS)
endif(WIN32)

FILE(GLOB_RECURSE SRC_CLNT "*.cpp" "*.h")

add_library(zmClient SHARED ${SRC_CLNT})
