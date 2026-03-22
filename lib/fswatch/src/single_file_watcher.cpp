#include <single_file_watcher.hpp>
#include <filesystem>

SingleFileWatcher::SingleFileWatcher(std::string const &single_file_name)
    : file_name(single_file_name) {
  auto dir_name =
      std::filesystem::path(single_file_name).parent_path().string();
  if (dir_name == "")
    dir_name = ".";
  add_watch_for_path(dir_name);
}

bool SingleFileWatcher::should_handle_file(Event event) const {
  return get_filename(event) == file_name;
}
