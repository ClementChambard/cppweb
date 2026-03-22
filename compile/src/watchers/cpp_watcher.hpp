#pragma once

#include <fswatch/recursive_file_watcher.hpp>

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
