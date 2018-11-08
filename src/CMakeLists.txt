cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

PROJECT(smey)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Builds/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Builds/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Builds/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME})

if(MSVC)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  link_directories(";$(OutDir)")
endif(MSVC)

# copy mswin dependent
if(WIN32)
  mswin_getDependDir(DEPEND_WIN_DIR)
  add_custom_command(TARGET skynet PRE_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPEND_WIN_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
