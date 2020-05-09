cmake_minimum_required(VERSION 3.0.2)

project(zmAuxFunc)

FILE(GLOB_RECURSE SRC_zmAux "src/*.cpp" "src/*.h")

add_library(zmAuxFunc STATIC ${SRC_zmAux})