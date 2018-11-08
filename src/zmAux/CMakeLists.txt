cmake_minimum_required(VERSION 3.0.2)

project(zmAux)

FILE(GLOB_RECURSE SRC_Aux "*.cpp" "*.h")

if (UNIX)
   add_compile_options(-fPIC )
endif(UNIX)

add_library(zmAux STATIC ${SRC_Aux})