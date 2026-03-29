cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(api ${SRCS} ${HEADERS})

set_property(TARGET api PROPERTY POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(api PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(api PRIVATE -fsanitize=address -fno-gnu-unique)
  target_link_options(api PRIVATE -fsanitize=address -fno-gnu-unique)
else()
  target_compile_definitions(api PRIVATE -DNS_RELEASE=1)
endif()

target_link_libraries(api PRIVATE system)
target_link_libraries(api PUBLIC http)
target_link_libraries(api PUBLIC jsoncpp)

target_include_directories(api PRIVATE include/api)
target_include_directories(api INTERFACE include)

