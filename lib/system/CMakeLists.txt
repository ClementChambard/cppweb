cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(system ${SRCS} ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(system PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(system PRIVATE -fsanitize=address -fno-gnu-unique)
  target_link_options(system PRIVATE -fsanitize=address -fno-gnu-unique)
else()
  target_compile_definitions(system PRIVATE -DNS_RELEASE=1)
endif()

target_include_directories(system PRIVATE include/sys)
target_include_directories(system INTERFACE include)
