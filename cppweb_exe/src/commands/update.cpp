#include "../commands.hpp"
#include "../fs_helpers.hpp"
#include <filesystem>

void update_cppweb(Args::Update const &args, bool nobuild) {
  auto cppweb_dir = get_cppweb_dir();

  if (args.bin_only) {
    std::filesystem::remove_all(".cppweb/bin");
    std::filesystem::create_directory(".cppweb/bin");
    cp_echo(cppweb_dir + "/build/server/server", ".cppweb/bin");
    cp_echo(cppweb_dir + "/build/compile/compile", ".cppweb/bin");
    return;
  }

  for (auto const &d : std::filesystem::directory_iterator(".cppweb")) {
    std::filesystem::remove_all(d);
    std::filesystem::remove_all(d);
    std::filesystem::remove_all(d);
    std::filesystem::remove_all(d);
  }
  std::filesystem::create_directory(".cppweb/bin");
  std::filesystem::create_directory(".cppweb/lib");
  std::filesystem::create_directory(".cppweb/include");
  std::filesystem::create_directory(".cppweb/pages");

  // BINARIES
  cp_echo(cppweb_dir + "/build/server/server", ".cppweb/bin");
  cp_echo(cppweb_dir + "/build/compile/compile", ".cppweb/bin");

  // TODO: depends on 'modules'

  // LIBRARIES
  cp_echo(cppweb_dir + "/build/lib/http/libhttp.a", ".cppweb/lib");
  cp_echo(cppweb_dir + "/build/lib/api/libapi.a", ".cppweb/lib");
  cp_echo(cppweb_dir + "/build/lib/system/libsystem.a", ".cppweb/lib");
  cp_echo(cppweb_dir + "/build/lib/db/libdb.a", ".cppweb/lib");
  cp_echo(cppweb_dir + "/build/lib/cppweb/libcppweb.a", ".cppweb/lib");

  // INCLUDES
  cp_echo(cppweb_dir + "/common/defines.hpp", ".cppweb/include");
  cp_echo(cppweb_dir + "/common/cppweb_init.hpp", ".cppweb/include");
  cp_echo(cppweb_dir + "/common/cppweb_static_component.hpp",
          ".cppweb/include");
  cp_echo(cppweb_dir + "/common/cppweb_server_component.hpp",
          ".cppweb/include");
  cp_dir_echo(cppweb_dir + "/lib/http/include/http", ".cppweb/include");
  cp_dir_echo(cppweb_dir + "/lib/api/include/api", ".cppweb/include");
  cp_dir_echo(cppweb_dir + "/lib/system/include/sys", ".cppweb/include");
  cp_dir_echo(cppweb_dir + "/lib/db/include/db", ".cppweb/include");
  cp_dir_echo(cppweb_dir + "/lib/cppweb/include/cppweb", ".cppweb/include");

  if (!nobuild)
    build_project();
}
