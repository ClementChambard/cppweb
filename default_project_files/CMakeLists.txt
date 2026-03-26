cmake_minimum_required(VERSION 4.2.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")

add_compile_options(-Wall -Wextra -Werror -fno-gnu-unique)

project(website VERSION 1.0)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(website SHARED ${SRCS} ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(website PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(website PRIVATE -fsanitize=address)
  target_link_options(website PRIVATE -fsanitize=address)
else()
  target_compile_definitions(website PRIVATE -DNS_RELEASE=1)
endif()

target_include_directories(website PRIVATE ${CMAKE_SOURCE_DIR}/.cppweb/include)
target_link_directories(website PRIVATE ${CMAKE_SOURCE_DIR}/.cppweb/lib)

target_link_libraries(website PRIVATE api html system)
