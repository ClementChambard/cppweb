cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(fswatch ${SRCS} ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(fswatch PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(fswatch PRIVATE -fsanitize=address -fno-gnu-unique)
  target_link_options(fswatch PRIVATE -fsanitize=address -fno-gnu-unique)
else()
  target_compile_definitions(fswatch PRIVATE -DNS_RELEASE=1)
endif()

target_include_directories(fswatch PRIVATE include/fswatch)
target_include_directories(fswatch INTERFACE include)
