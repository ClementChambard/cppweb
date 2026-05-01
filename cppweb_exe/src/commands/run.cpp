#include "../commands.hpp"
#include <sys/subprocess.hpp>

void run(Args::Run const &args) {
  compile_project();
  if (args.port) {
    sys::export_env_var("PORT", *args.port);
  }
  if (args.run_mode != Args::NORMAL) {
    auto rm = Args::run_mode_str(args.run_mode);
    sys::exe_run(".cppweb/bin/compile", {"-dev", "-c", "cppweb.conf"});
    sys::exe_run(".cppweb/bin/server", {std::string(rm)}, false);
  } else {
    sys::exe_run(".cppweb/bin/server", {}, false);
  }
}
