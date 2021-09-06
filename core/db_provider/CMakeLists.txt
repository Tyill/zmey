cmake_minimum_required(VERSION 3.0.2)

project(db_provider)

FILE(GLOB_RECURSE SRC_zmDb "*.cpp" "*.h")

add_library(db_provider STATIC ${SRC_zmDb})

if (UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

find_package(PostgreSQL REQUIRED)

target_include_directories(db_provider PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(db_provider PRIVATE ${PostgreSQL_LIBRARIES} common)