cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_library(cppweb ${SRCS} ${HEADERS})

set_property(TARGET cppweb PROPERTY POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(cppweb PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(cppweb PRIVATE -fsanitize=address -fno-gnu-unique)
  target_link_options(cppweb PRIVATE -fsanitize=address -fno-gnu-unique)
else()
  target_compile_definitions(cppweb PRIVATE -DNS_RELEASE=1)
endif()

target_include_directories(cppweb PRIVATE include/cppweb)
target_include_directories(cppweb INTERFACE include)

target_link_libraries(cppweb PRIVATE jsoncpp)
