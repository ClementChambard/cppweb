#include "cmakelists.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

// clang-format off
void generate_cmake_lists(Config const &c) {
  std::ofstream ofs("build/CMakeLists.txt");

  ofs << "cmake_minimum_required(VERSION 4.2.3)\n";
  ofs << "set(CMAKE_CXX_STANDARD " << c.cpp.standard.value_or("23") << ")\n";
  ofs << "set(CMAKE_CXX_STANDARD_REQUIRED True)\n";
  ofs << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
  if (c.cpp.default_debug.value_or(true)) {
    ofs << "set(CMAKE_BUILD_TYPE \"Debug\")\n";
  } else {
    ofs << "set(CMAKE_BUILD_TYPE \"Release\")\n";
  }
  ofs << "set(SRC_DIR ${CMAKE_SOURCE_DIR}/..)\n";
  if (c.cpp.compile_options.size() > 0) {
    ofs << "add_compile_options(";
    bool f = false;
    for (auto const &o : c.cpp.compile_options) {
      if (f) ofs << ' ';
      f = true;
      ofs << o;
    }
    ofs << ")\n";
  }
  if (c.cpp.link_options.size() > 0) {
    ofs << "add_link_options(";
    bool f = false;
    for (auto const &o : c.cpp.link_options) {
      if (f) ofs << ' ';
      f = true;
      ofs << o;
    }
    ofs << ")\n";
  }
  ofs << "project(" << c.cpp.lib_name << " VERSION " << c.version.value_or("1.0.0") << ")\n";
  ofs << "file(GLOB_RECURSE SRCS ${SRC_DIR}/" << c.cpp.src_dir << "/*.cpp ${SRC_DIR}/" << c.cpp.src_dir << "/*.c)\n";
  ofs << "file(GLOB_RECURSE HEADERS ${SRC_DIR}/" << c.cpp.src_dir << "/*.hpp ${SRC_DIR}/" << c.cpp.src_dir << "/*.h)\n";
  ofs << "add_library(" << c.cpp.lib_name << " SHARED ${SRCS} ${HEADERS})\n";
  ofs << "if(CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
  ofs << "target_compile_definitions(" << c.cpp.lib_name << " PRIVATE -DNS_RELEASE=0 -D_DEBUG)\n";
  ofs << "target_compile_options(" << c.cpp.lib_name << " PRIVATE -fno-gnu-unique";
  if (c.cpp.use_asan.value_or(false)) ofs << " -fsanitize=address";
  ofs << ")\n";
  ofs << "target_link_options(" << c.cpp.lib_name << " PRIVATE -fno-gnu-unique";
  if (c.cpp.use_asan.value_or(false)) ofs << " -fsanitize=address";
  ofs << ")\n";
  ofs << "else()\n";
  ofs << "target_compile_definitions(" << c.cpp.lib_name << " PRIVATE -DNS_RELEASE=1)\n";
  ofs << "endif()\n";
  ofs << "target_include_directories(" << c.cpp.lib_name << " PRIVATE ${SRC_DIR}/.cppweb/include";
  for (auto const &o : c.cpp.include_dirs) {
    ofs << " " << o;
  }
  ofs << ")\n";
  if (c.cpp.libraries.empty() && c.cpp.modules.empty()) return;
  ofs << "target_link_directories(" << c.cpp.lib_name << " PRIVATE";
  if (!c.cpp.modules.empty()) ofs << " ${SRC_DIR}/.cppweb/lib";
  for (auto const &o : c.cpp.link_dirs) {
    ofs << " " << o;
  }
  ofs << ")\n";
  ofs << "target_link_libraries(" << c.cpp.lib_name << " PRIVATE";
  for (auto const &o : c.cpp.modules) {
    ofs << " " << o;
  }
  for (auto const &o : c.cpp.libraries) {
    ofs << " " << o;
  }
  ofs << ")\n";
  ofs.close();
}

void maybe_rebuild_cmakelists() {
  if (!std::filesystem::exists("build/CMakeLists.txt") || std::filesystem::last_write_time("cppweb.conf") > std::filesystem::last_write_time("build/CMakeLists.txt")) {
    std::cout << "Regenerating CMakeLists.txt\n";
    Config c("cppweb.conf");
    generate_cmake_lists(c);
  }
}
