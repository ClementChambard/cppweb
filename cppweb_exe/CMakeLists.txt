cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)

add_executable(cppweb_exe ${SRCS} ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(cppweb_exe PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(cppweb_exe PRIVATE -fsanitize=address)
  target_link_options(cppweb_exe PRIVATE -fsanitize=address)
else()
  target_compile_definitions(cppweb_exe PRIVATE -DNS_RELEASE=1)
endif()

target_link_libraries(cppweb_exe PRIVATE cppweb system)

add_custom_target(actual_exe ALL COMMAND cp ${CMAKE_BINARY_DIR}/cppweb_exe/cppweb_exe ${CMAKE_SOURCE_DIR}/bin/cppweb
                  BYPRODUCTS ${CMAKE_SOURCE_DIR}/bin/cppweb
                  DEPENDS cppweb_exe)

