cmake_minimum_required(VERSION 3.0.2)

PROJECT("zmey")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") 

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

if(WIN32)         
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_subdirectory(common)
add_subdirectory(base)
add_subdirectory(db_provider)
add_subdirectory(scheduler)
add_subdirectory(worker)
add_subdirectory(client)

find_package(GTest)
if (GTest_FOUND)
  add_subdirectory(test)
endif()
