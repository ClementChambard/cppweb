cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_executable(compile ${SRCS} ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(compile PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(compile PRIVATE -fsanitize=address)
  target_link_options(compile PRIVATE -fsanitize=address)
else()
  target_compile_definitions(compile PRIVATE -DNS_RELEASE=1)
endif()

target_link_libraries(compile PRIVATE html jsoncpp system fswatch http cppweb)
