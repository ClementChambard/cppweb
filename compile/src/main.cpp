#include "args.hpp"
#include "component_library.hpp"
#include "page.hpp"
#include "sys/logger.hpp"
#include <cppweb/config.hpp>

void start_watcher_loop();

void run_config() {
  ComponentLibrary lib(CONFIG.cpp_bin());

  build_all_pages(CONFIG.pages.dir, CONFIG.pages.build_dir);
}

int main(int argc, char **argv) {
  Args args;
  args.parse(argc, argv);

  if (args.config_name != "") {
    CONFIG = Config(args.config_name);
    sys::load_logger_config();
    CONFIG.dev = args.dev;
    run_config();
  } /*else if (args.mode == Args::FILE) {
    run_file(args);
  }*/
  else if (args.mode == Args::DIRECTORY) {
    ComponentLibrary lib(args.component_library);
    CONFIG.dev = args.dev;
    build_all_pages(args.input_name, args.output_name);
  } else {
    sys::fatal_error("mode not implemented");
  }

  return 0;
}
