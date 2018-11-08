cmake_minimum_required(VERSION 3.0.2)

project(zmBase)

FILE(GLOB_RECURSE SRC_BASE "*.cpp" "*.h")

add_library(zmBase INTERFACE SRC_BASE)
