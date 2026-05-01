#include "compiled_format.hpp"
#include "http_server.hpp"
#include "loaded_data.hpp"
#include "ws_connection.hpp"
#include <api/api.hpp>
#include <cppweb/config.hpp>
#include <cppweb/server_rendering_context.hpp>
#include <filesystem>
#include <fstream>
#include <http/request.hpp>
#include <http/response.hpp>
#include <iostream>
#include <sys/env.hpp>
#include <sys/logger.hpp>

api::Api *THE_API;

void create_routes(Router &router) {
  router.forward(CONFIG.api.root.c_str(), THE_API->get_func());
  THE_API->base_url = CONFIG.api.root;

  if (CONFIG.dev)
    ws_make_route(router);

  for (auto const &p :
       std::filesystem::recursive_directory_iterator(CONFIG.pages.build_dir)) {
    std::filesystem::path filename = p.path();
    std::string path = filename.filename().string();
    sys::log_extra("ROUTE", "registered page: %s", path.c_str());
    router.page(path.c_str(), [filename, path](http::Request r) {
      cppweb::ServerRenderingContext ctx;
      ctx.page_params = std::move(r.params);
      ctx.query_params = std::move(r.query);
      ctx.current_page = path;
      if (auto it = r.headers.find("Cookie"); it != r.headers.end()) {
        ctx.set_cookies(it->second);
      }
      if (LOADED_SO.instanciate_context) {
        ctx.user_data = LOADED_SO.instanciate_context(ctx);
      }
      std::ifstream f(filename);
      std::ostringstream oss;
      oss << f.rdbuf();
      std::string build_body = exec_compiled_format(ctx, oss.str());
      std::vector<u8> body = ctx.get_body(build_body);
      if (ctx.user_data != nullptr) {
        LOADED_SO.cleanup_context(ctx.user_data);
      }
      return http::Response::Builder()
          .body(ctx.response_type, body)
          .code(200)
          .build();
    });
  }
}

void start_watcher_loop();

int main(int argc, char **argv) {
  if (std::filesystem::exists("cppweb.conf")) {
    CONFIG = Config("cppweb.conf");
    sys::load_logger_config();
  }

  if (argc > 2) {
    sys::fatal_error("invalid argument count");
  }

  if (argc == 2) {
    std::string mode = argv[1];
    if (mode != "dev") {
      sys::fatal_error("invalid run mode: %s", mode.c_str());
    }
    CONFIG.dev = true;
  }

  sys::log_extra("TEST", "test extra enabled!");

  // TODO: better
  LOADED_SO.load(CONFIG.cpp_bin());

  auto port = std::stoi(sys::get_env_var("PORT", "8080"));

  auto server = HttpServer("0.0.0.0", port);

  create_routes(server.router());

  if (CONFIG.dev) {
    server.start_listen(true);
    ws_open_connection(&server);
    start_watcher_loop();
    ws_send_string("{\"action\":\"close\"}");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    server.stop();
  } else {
    server.start_listen();
  }

  LOADED_SO.close();

  return 0;
}
