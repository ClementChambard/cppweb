#include "args.hpp"
#include "components.hpp"
#include "cppweb/logger.hpp"
#include "hot_reload_socket.hpp"
#include "html/components.hpp"
#include "html/parse.hpp"
#include "page.hpp"
#include "sys/logger.hpp"
#include "ws_connection.hpp"
#include <cppweb/config.hpp>
#include <fstream>

Config CONFIG;

void start_watcher_loop();

void run_file(Args const &args) {
  MapH map(args.component_library);

  std::ifstream input(args.input_name);

  std::ostringstream oss;
  oss << input.rdbuf();
  components::Context ctx;
  auto content = html::parse(oss.str(), &ctx);

  std::ofstream out(args.output_name);

  out << "<!DOCTYPE html>" << html::node_str(content);
}

void run_config() {
  MapH map(CONFIG.cpp_bin());

  build_all_pages();

  if (CONFIG.dev) {
    hot_reload_socket_create();
    do_hot_reload();
    ws_open_connection();
    start_watcher_loop();
    ws_close_connection();
    hot_reload_socket_close();
  }
}

int main(int argc, char **argv) {
  sys::setup_logger(logger::info, logger::warn, logger::error,
                    logger::fatal_error, logger::log_extra);

  Args args;
  args.parse(argc, argv);

  if (args.config_name != "") {
    CONFIG = Config(args.config_name);
    if (CONFIG.logger)
      logger::set_config(*CONFIG.logger);
    CONFIG.dev = args.dev;
    run_config();
  } else if (args.mode != Args::FILE) {
    logger::fatal_error("Directory mode not implemented");
  } else {
    run_file(args);
  }

  return 0;
}
