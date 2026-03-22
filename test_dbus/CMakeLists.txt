cmake_minimum_required(VERSION 4.2.3)

file(GLOB_RECURSE S_SRCS send/*.cpp src/*.c)
file(GLOB_RECURSE S_HEADERS send/*.hpp src/*.h)
file(GLOB_RECURSE R_SRCS receive/*.cpp src/*.c)
file(GLOB_RECURSE R_HEADERS receive/*.hpp src/*.h)

add_executable(test_dbus_send ${S_SRCS} ${S_HEADERS})
add_executable(test_dbus_receive ${R_SRCS} ${R_HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(test_dbus_send PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(test_dbus_send PRIVATE -fsanitize=address)
  target_link_options(test_dbus_send PRIVATE -fsanitize=address)
  target_compile_definitions(test_dbus_receive PRIVATE -DNS_RELEASE=0 -D_DEBUG)
  target_compile_options(test_dbus_receive PRIVATE -fsanitize=address)
  target_link_options(test_dbus_receive PRIVATE -fsanitize=address)
else()
  target_compile_definitions(test_dbus_send PRIVATE -DNS_RELEASE=1)
  target_compile_definitions(test_dbus_receive PRIVATE -DNS_RELEASE=1)
endif()

target_link_libraries(test_dbus_send PRIVATE dbus-c++-1)
target_link_libraries(test_dbus_receive PRIVATE dbus-c++-1)
