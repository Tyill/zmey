cmake_minimum_required(VERSION 3.0.2)

project(zmCommon)

FILE(GLOB_RECURSE SRC_zmCmn "*.cpp" "*.h")

add_library(zmCommon STATIC ${SRC_zmCmn})