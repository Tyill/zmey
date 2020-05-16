cmake_minimum_required(VERSION 3.0.2)

project(zmDbProvider)

FILE(GLOB_RECURSE SRC_zmDb "*.cpp" "*.h")

add_library(zmDbProvider STATIC ${SRC_zmDb})

# find_package(PostgreSQL REQUIRED)

# target_include_directories(zmDbProvider PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(zmDbProvider PRIVATE zmCommon sqlite3 dl)
# target_link_libraries(zmDbProvider PRIVATE ${PostgreSQL_LIBRARIES} zmCommon)