#pragma once

#include <fswatch/single_file_watcher.hpp>
#include <fswatch/recursive_file_watcher.hpp>
#include <sys/inotify.h>
#include <iostream>
#include <sys/subprocess.hpp>
#include "ws_connection.hpp"

struct CppWatcher : RecursiveFileWatcher {
  CppWatcher(std::string const &p) : RecursiveFileWatcher(p) {}

  void handle_event(Event e) override {
    if ((e.event->mask & (IN_CREATE | IN_DELETE | IN_CLOSE_WRITE)) == 0)
      return;
    auto fn = get_filename(e);
    if (fn.ends_with(".cpp") || fn.ends_with(".hpp") || fn.ends_with(".c") ||
        fn.ends_with(".h")) {
      std::cout << fn << '\n';
      char const *args_[] = {"cppweb", "compile"};
      sys::subprocess_run(args_);
    }
  }
};

struct SoWatcher : SingleFileWatcher {
  SoWatcher(std::string const &f) : SingleFileWatcher(f) {}

  void handle_event(Event e) override {
    if (!should_handle_file(e))
      return;
    if ((e.event->mask & IN_CLOSE_WRITE) == 0)
      return;
    ws_send_string("{\"changed_pages\": [\"/\"]}");
  }
};
