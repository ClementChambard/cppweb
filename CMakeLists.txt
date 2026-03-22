cmake_minimum_required(VERSION 4.2.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")

add_compile_options(-Wall -Wextra -Werror)
include_directories(common)

project(cppweb VERSION 1.1)

add_subdirectory(compile)
add_subdirectory(lib)
add_subdirectory(server)
add_subdirectory(test_dbus)
