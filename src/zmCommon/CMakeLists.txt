cmake_minimum_required(VERSION 3.0.2)

project(zmCommon)

FILE(GLOB_RECURSE SRC_zmCmn "src/*.cpp" "src/*.h")

add_library(zmCommon STATIC ${SRC_zmCmn})