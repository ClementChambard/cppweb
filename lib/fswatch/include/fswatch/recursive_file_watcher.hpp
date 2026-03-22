#pragma once

#include "watcher.hpp"

struct RecursiveFileWatcher : Watcher {
  RecursiveFileWatcher(std::string const &root_dir);

  void handle_event(Event event) override;
};
