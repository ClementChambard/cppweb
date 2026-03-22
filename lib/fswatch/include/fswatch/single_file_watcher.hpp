#pragma once

#include "watcher.hpp"

struct SingleFileWatcher : Watcher {
  std::string file_name;
  SingleFileWatcher(std::string const &single_file_name);
  bool should_handle_file(Event event) const;
};
