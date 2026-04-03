#include "../commands.hpp"
#include <sys/subprocess.hpp>

void build_project() {
  compile_project();
  sys::exe_run(".cppweb/bin/compile", {"-c", "cppweb.conf"}, false);
}
