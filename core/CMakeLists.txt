cmake_minimum_required(VERSION 3.0.2)

PROJECT("zmey")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})
add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") 

if(MSVC)   
   add_definitions(-DWIN32_LEAN_AND_MEAN)
   add_definitions(-D_SCL_SECURE_NO_WARNINGS)
   add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   link_directories(";$(OutDir)")
endif(MSVC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

add_subdirectory(zmCommon)
add_subdirectory(zmDbProvider)
add_subdirectory(zmScheduler)
add_subdirectory(zmWorker)
add_subdirectory(zmClient)
add_subdirectory(zmTest)