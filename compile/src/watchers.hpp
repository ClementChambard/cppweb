#pragma once

#include "hot_reload_socket.hpp"
#include "ws_connection.hpp"
#include <fswatch/recursive_file_watcher.hpp>
#include <fswatch/single_file_watcher.hpp>
#include <iostream>
#include <sys/inotify.h>
#include <sys/subprocess.hpp>
#include <thread>

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
    // auto diff = rebuild_pages();
    do_hot_reload();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ws_send_string("{\"changed_pages\": [\"/\"]}");
  }
};

struct ResourceWatcher : RecursiveFileWatcher {
  std::string base_path;
  ResourceWatcher(std::string const &p)
      : RecursiveFileWatcher(p), base_path(p) {}

  void handle_event(Event e) override {
    if ((e.event->mask & (IN_CREATE | IN_DELETE | IN_CLOSE_WRITE)) == 0)
      return;
    auto fn = get_filename(e);
    if (fn.starts_with(base_path)) {
      fn = fn.substr(base_path.size());
    }
    // TODO: more file types
    if (fn.ends_with(".css") || fn.ends_with(".js")) {
      ws_send_string("{\"changed_res\": \"" + fn + "\"}");
    }
  }
};
