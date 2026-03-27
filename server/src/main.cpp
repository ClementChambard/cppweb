#include "html/components.hpp"
#include "html/server_rendering_context.hpp"
#include <api/api.hpp>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
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
#include <sys/poll.h>
#include <sys/socket.h>

struct Config {
  std::string base_api_route = "/api";
  std::string website_dll = "build/libwebsite.so";
  std::string page_build_dir = "";
};

template <typename pfn_t>
pfn_t *get_func(void *so, std::string const &name, bool can_be_empty = false) {
  void *pfn = dlsym(so, name.c_str());

  if (pfn == nullptr && !can_be_empty) {
    std::cerr << "Error accessing function " << name << " - " << dlerror()
              << '\n';
    std::exit(EXIT_FAILURE);
  }

  return reinterpret_cast<pfn_t *>(pfn);
}

#include <filesystem>

bool DEV = false;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;
extern components::Map *REGISTERED_COMPONENTS;
static api::Api *API;

struct LoadedData {
  using fn_get_api_t = api::Api *();
  using fn_init_api_t = void();
  using fn_delete_api_t = void();
  using fn_get_registered_server_components_t = components::ServerMap *();
  using fn_get_registered_components_t = components::Map *();
  using fn_instanciate_context_t =
      void *(html::ServerRenderingContext const &ctx);
  using fn_cleanup_context_t = void(void *);

  std::string so_file;
  void *so = nullptr;

  fn_get_api_t *get_api = nullptr;
  fn_init_api_t *init_api = nullptr;
  fn_delete_api_t *delete_api = nullptr;
  fn_get_registered_server_components_t *get_registered_server_components =
      nullptr;
  fn_get_registered_components_t *get_registered_components = nullptr;
  fn_instanciate_context_t *instanciate_context = nullptr;
  fn_cleanup_context_t *cleanup_context = nullptr;

  void load(std::string const &file);
  void hot_reload();
  void close();

  ~LoadedData() {
    if (so)
      close();
  }
};

void LoadedData::hot_reload() {
  auto so = so_file;
  close();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  load(so);
}

void LoadedData::load(std::string const &file) {
  so_file = file;
  so = dlopen(file.c_str(), RTLD_LAZY | RTLD_LOCAL);

  if (so == nullptr) {
    std::cerr << "Error loading library " << file << " - " << dlerror() << '\n';
    std::exit(EXIT_FAILURE);
  }

  dlerror();

  get_api = get_func<fn_get_api_t>(so, "get_api");
  init_api = get_func<fn_init_api_t>(so, "init_api");
  delete_api = get_func<fn_delete_api_t>(so, "delete_api");
  get_registered_components =
      get_func<fn_get_registered_components_t>(so, "get_registered_components");
  get_registered_server_components =
      get_func<fn_get_registered_server_components_t>(
          so, "get_registered_server_components");
  instanciate_context =
      get_func<fn_instanciate_context_t>(so, "instanciate_context", true);
  if (instanciate_context)
    cleanup_context = get_func<fn_cleanup_context_t>(so, "cleanup_context");

  init_api();

  API = get_api();
  REGISTERED_SERVER_COMPONENTS = get_registered_server_components();
  REGISTERED_COMPONENTS = get_registered_components();
}

void LoadedData::close() {
  delete_api();
  delete REGISTERED_SERVER_COMPONENTS;
  delete REGISTERED_COMPONENTS;
  dlclose(so);
  so_file = "";
  so = nullptr;
  get_api = nullptr;
  init_api = nullptr;
  delete_api = nullptr;
  get_registered_server_components = nullptr;
  instanciate_context = nullptr;
  cleanup_context = nullptr;
}

LoadedData LOADED_SO{};

Config CONFIG;

void create_routes(http::Router &router) {
  router.forward(CONFIG.base_api_route.c_str(), API->get_func());

  for (auto const &p :
       std::filesystem::recursive_directory_iterator(CONFIG.page_build_dir)) {
    std::filesystem::path filename = p.path();
    std::string path = filename.filename().string();
    std::cout << "found: " << path << '\n';
    router.page(path.c_str(), [filename, path](http::Request r) {
      html::ServerRenderingContext ctx;
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
      std::string body = html::exec_compiled_format(ctx, oss.str());
      ctx.get_special_html(body);
      if (ctx.user_data != nullptr) {
        LOADED_SO.cleanup_context(ctx.user_data);
      }
      return http::Response::Builder()
          .body("text/html", body)
          .code(200)
          .build();
    });
  }
}

void hot_reload_server(http::HttpServer &serv) {
  auto pid = fork();
  if (pid == 0) {
    char const *sp_args[] = {".cppweb/bin/compile", "-c", "cppweb.conf", "-dev",
                             0};
    execv(".cppweb/bin/compile", const_cast<char *const *>(sp_args));
  }
  auto sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return;
  sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    return;
  if (listen(sock, 1) < 0)
    return;
  socklen_t client_addr_len;
  sockaddr_in client_addr;
  auto client_sock = accept(sock, (sockaddr *)&client_addr, &client_addr_len);
  if (client_sock < 0)
    return;
  nfds_t nfds = 2;
  struct pollfd fds[2];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = client_sock;
  fds[1].events = POLLIN;
  while (true) {
    auto poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }
    if (poll_num > 0) {
      if (fds[0].revents & POLLIN) {
        http::ServerCommand cmd;
        std::getline(std::cin, cmd.cmd, '\n');
        cmd.parse();
        if (http::execute_server_command(serv, cmd))
          break;
      }
      if (fds[1].revents & POLLIN) {
        // receive anything => hot reload
        char buf;
        read(client_sock, &buf, 1);

        serv.router().clean();
        LOADED_SO.hot_reload();
        create_routes(serv.router());
      }
    }
  }
}

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

  if (std::filesystem::exists("cppweb.conf")) {
    std::ifstream f{"cppweb.conf"};
    Json::Reader r;
    Json::Value root;
    r.parse(f, root);

    if (root.isMember("api")) {
      Json::Value &api = root["api"];
      if (api.isMember("root")) {
        CONFIG.base_api_route = api["root"].asString();
      }
    }
    if (root.isMember("pages")) {
      Json::Value &api = root["pages"];
      if (api.isMember("build_dir")) {
        CONFIG.page_build_dir = api["build_dir"].asString();
      }
    }

    // TODO:...
  }

  // TODO: better
  LOADED_SO.load(CONFIG.website_dll);

  auto port = std::stoi(sys::get_env_var("PORT", "8080"));

  auto server = http::HttpServer("0.0.0.0", port);

  create_routes(server.router());

  if (DEV) {
    server.start_listen(true);
    hot_reload_server(server);
  } else {
    server.start_listen();
  }

  LOADED_SO.close();

  return 0;
}
