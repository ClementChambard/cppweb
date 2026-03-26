cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(http ${SRCS} ${HEADERS})

set_property(TARGET http PROPERTY POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(http PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(http PRIVATE -fsanitize=address)
  target_link_options(http PRIVATE -fsanitize=address)
else()
  target_compile_definitions(http PRIVATE -DNS_RELEASE=1)
endif()

target_link_libraries(http PRIVATE system)
target_link_libraries(http PUBLIC jsoncpp)

target_include_directories(http PRIVATE include/http)
target_include_directories(http INTERFACE include)

