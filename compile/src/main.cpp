#include "args.hpp"
#include "components.hpp"
#include "config.hpp"
#include "hot_reload_socket.hpp"
#include "html/parse.hpp"
#include "page.hpp"
#include "sys/logger.hpp"
#include "ws_connection.hpp"
#include <fstream>

void start_watcher_loop();

void run_file(Args const &args) {
  MapH map(args.component_library);

  std::ifstream input(args.input_name);

  std::ostringstream oss;
  oss << input.rdbuf();
  auto content = html::parse(oss.str());

  std::ofstream out(args.output_name);

  out << "<!DOCTYPE html>" << html::node_str(content);
}

void run_config() {
  MapH map(CONFIG.cpp_bin);

  auto pages = Page::find_all();

  for (auto const &p : pages) {
    p.compile();
  }

  if (CONFIG.dev) {
    hot_reload_socket_create();
    do_hot_reload();
    ws_open_connection();
    start_watcher_loop();
    hot_reload_socket_close();
  }
}

int main(int argc, char **argv) {

  Args args;
  args.parse(argc, argv);

  if (args.config_name != "") {
    CONFIG = Config(args.config_name);
    CONFIG.dev = args.dev;
    run_config();
  } else if (args.mode != Args::FILE) {
    sys::fatal_error("Directory mode not implemented");
  } else {
    run_file(args);
  }

  return 0;
}
