#include "../cmakelists.hpp"
#include "../commands.hpp"
#include "cppweb/config.hpp"
#include <filesystem>
#include <sys/subprocess.hpp>

void compile_project() {
  maybe_rebuild_cmakelists();
  Config c("cppweb.conf");
  std::filesystem::current_path(c.cpp.build_dir);
  sys::exe_run("cmake", {".."});
  sys::exe_run("ninja");
  std::filesystem::current_path("..");
}
