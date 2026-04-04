#include "../cmakelists.hpp"
#include "../commands.hpp"
#include "../fs_helpers.hpp"
#include <filesystem>
#include <sys/subprocess.hpp>

void create_project(Args::Create const &args) {
  auto cppweb_dir = get_cppweb_dir(false);
  // TODO: get absolute dir
  auto cppweb_dir_absolute = cppweb_dir;
  auto cwd = std::filesystem::current_path();
  std::filesystem::copy(cppweb_dir_absolute + "/default_project_files",
                        cwd.string() + "/" + std::string(args.project_name),
                        std::filesystem::copy_options::recursive);
  std::filesystem::current_path(std::string(args.project_name));
  Config conf("cppweb.conf");
  conf.cppweb.dir = cppweb_dir_absolute;
  // TODO: only do this when name has no invalid characters
  conf.cpp.lib_name = args.project_name;
  if (args.build_dir)
    conf.cpp.build_dir = std::string(*args.build_dir);
  conf.write("cppweb.conf");
  generate_cmake_lists(conf);
  std::filesystem::create_directory(".cppweb");
  update_cppweb(true);
  std::filesystem::create_directory(conf.cpp.build_dir);
  std::filesystem::current_path(conf.cpp.build_dir);
  // TODO: builder
  sys::exe_run("cmake", {".", "-GNinja"});
  sys::exe_run("ninja");
  std::filesystem::current_path("..");
  sys::exe_run(".cppweb/bin/compile", {"-c", "cppweb.conf"}, false);
}
