cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(db ${SRCS} ${HEADERS})

set_property(TARGET db PROPERTY POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(db PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(db PRIVATE -fsanitize=address -fno-gnu-unique)
  target_link_options(db PRIVATE -fsanitize=address -fno-gnu-unique)
else()
  target_compile_definitions(db PRIVATE -DNS_RELEASE=1)
endif()

target_include_directories(db PRIVATE include/db)
target_include_directories(db INTERFACE include)
target_link_libraries(db PUBLIC sqlite3 system)
