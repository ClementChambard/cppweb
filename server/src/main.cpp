#include "html/components.hpp"
#include <api/api.hpp>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <html/compiled_format.hpp>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/tcp_server.hpp>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <sstream>
#include <sys/env.hpp>
#include <sys/logger.hpp>

struct Config {
  std::string base_api_route = "/api";
  std::string website_dll = "build/libwebsite.so";
  std::string page_build_dir = "";
};

template <typename pfn_t> pfn_t *get_func(void *so, std::string const &name) {
  void *pfn = dlsym(so, name.c_str());

  if (pfn == nullptr) {
    std::cerr << "Error accessing function " << name << " - " << dlerror()
              << '\n';
    std::exit(EXIT_FAILURE);
  }

  return reinterpret_cast<pfn_t *>(pfn);
}

#include <filesystem>

bool DEV = false;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;

int main(int argc, char **argv) {

  if (argc > 2) {
    sys::fatal_error("invalid argument count");
  }
  if (argc == 2) {
    std::string mode = argv[1];
    if (mode != "dev") {
      sys::fatal_error("invalid run mode: %s", mode.c_str());
    }
    DEV = true;
  }

  Config c;

  if (DEV) {
    auto pid = fork();
    if (pid == 0) {
      char const *sp_args[] = {".cppweb/bin/compile", "-c", "cppweb.conf",
                               "-dev", 0};
      execv(".cppweb/bin/compile", const_cast<char *const *>(sp_args));
    }
  }

  if (std::filesystem::exists("cppweb.conf")) {
    std::ifstream f{"cppweb.conf"};
    Json::Reader r;
    Json::Value root;
    r.parse(f, root);

    if (root.isMember("api")) {
      Json::Value &api = root["api"];
      if (api.isMember("root")) {
        c.base_api_route = api["root"].asString();
      }
    }
    if (root.isMember("pages")) {
      Json::Value &api = root["pages"];
      if (api.isMember("build_dir")) {
        c.page_build_dir = api["build_dir"].asString();
      }
    }

    // TODO:...
  }

  // TODO: better
  std::string libfullname = c.website_dll;

  auto so = dlopen(libfullname.c_str(), RTLD_LAZY);

  if (so == nullptr) {
    std::cerr << "Error loading library " << libfullname << " - " << dlerror()
              << '\n';
    std::exit(EXIT_FAILURE);
  }

  dlerror();

  auto get_api = get_func<api::Api *()>(so, "get_api");
  auto init_api = get_func<void()>(so, "init_api");
  auto get_registered_server_components = get_func<components::ServerMap *()>(
      so, "get_registered_server_components");
  auto set_route_param =
      get_func<void(std::string const &, std::string const &)>(
          so, "set_route_param");

  init_api();
  auto api = get_api();
  REGISTERED_SERVER_COMPONENTS = get_registered_server_components();

  auto port = std::stoi(sys::get_env_var("PORT", "8080"));

  auto server = http::HttpServer("0.0.0.0", port);

  server.router().forward(c.base_api_route.c_str(), api->get_func());

  for (auto const &p :
       std::filesystem::recursive_directory_iterator(c.page_build_dir)) {
    std::filesystem::path filename = p.path();
    std::string path = filename.filename().string();
    std::cout << "found: " << path << '\n';
    server.router().page(
        path.c_str(), [filename, set_route_param, path](http::Request r) {
          for (auto &[k, v] : r.params)
            set_route_param(k, v);
          set_route_param("ROUTE", path);
          std::ifstream f(filename);
          std::ostringstream oss;
          oss << f.rdbuf();
          std::string body = html::exec_compiled_format(oss.str());
          return http::Response::Builder()
              .body("text/html", body)
              .code(200)
              .build();
        });
  }

  server.start_listen();

  delete api;
  dlclose(so);

  return 0;
}
