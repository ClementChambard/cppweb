struct InitCompilerData {
  void (*register_static_component)(
      struct CPPWEB_StaticComponentDecl const *component);
  void (*register_server_component)(char const *name);
};

extern "C" void init_compiler(InitCompilerData *init_data);
extern "C" void cleanup_compiler();

namespace api {
struct Api;
}
namespace components {
struct ServerComponent;
}

struct InitServerData {
  void (*register_server_component)(
      struct CPPWEB_ServerComponentDecl const *component);
  void (*register_api)(api::Api *api);
};

extern "C" void init_server(InitServerData *init_server);
extern "C" void cleanup_server();
