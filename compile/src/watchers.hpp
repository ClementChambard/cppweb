#pragma once

#include "components.hpp"
#include "hot_reload_socket.hpp"
#include "page.hpp"
#include "sys/logger.hpp"
#include "ws_connection.hpp"
#include <chrono>
#include <fswatch/recursive_file_watcher.hpp>
#include <fswatch/single_file_watcher.hpp>
#include <mutex>
#include <set>
#include <sys/inotify.h>
#include <sys/subprocess.hpp>
#include <thread>

struct Timeouter {
  using TimeoutFn = void(std::set<std::string> &&);
  using Duration = std::chrono::duration<int64_t, std::milli>;
  Timeouter(TimeoutFn *fn) : out_fn(fn) {}

  void set_timeout(std::string val) {
    mutex.lock();
    buf.insert(val);
    should_repeat.store(true);
    if (!running) {
      running.store(true);
      thread = std::thread(thread_func, this);
    }
    mutex.unlock();
  }

  static void thread_func(Timeouter *timeouter) {
    while (timeouter->should_repeat.load()) {
      timeouter->should_repeat.store(false);
      std::this_thread::sleep_for(timeouter->d);
    }
    timeouter->mutex.lock();
    if (timeouter->buf.size() != 0) {
      timeouter->out_fn(std::move(timeouter->buf));
      timeouter->buf.clear();
    }
    timeouter->running.store(false);
    timeouter->thread.detach();
    timeouter->mutex.unlock();
  }

  TimeoutFn *out_fn;
  Duration d = std::chrono::milliseconds(10);
  std::set<std::string> buf{};
  std::atomic<bool> should_repeat = false;
  std::atomic<bool> running = false;
  std::mutex mutex;
  std::thread thread;
};

struct CppWatcher : RecursiveFileWatcher {
  CppWatcher(std::string const &p) : RecursiveFileWatcher(p) {}

  Timeouter timeout{exec};

  static void exec(std::set<std::string> &&names) {
    std::string all_str;
    for (auto const &n : names)
      all_str += " " + n;
    sys::log_extra("COMPILE", "%s", all_str.substr(1).c_str());
    char const *args_[] = {"cppweb", "compile"};
    sys::subprocess_run(args_);
  }

  void handle_event(Event e) override {
    if ((e.event->mask & (IN_CREATE | IN_DELETE | IN_CLOSE_WRITE)) == 0)
      return;
    auto fn = get_filename(e);
    if (fn.ends_with(".cpp") || fn.ends_with(".hpp") || fn.ends_with(".c") ||
        fn.ends_with(".h")) {
      timeout.set_timeout(fn);
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
    // ws_send_string("{\"changed_pages\": [\"/\"]}");
    do_hot_reload();
    MapH::hot_reload();
    build_all_pages();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ws_send_string("{\"action\":\"refresh\"}");
  }
};

struct HtmlWatcher : RecursiveFileWatcher {
  HtmlWatcher(std::string const &p) : RecursiveFileWatcher(p) {}

  static void exec(std::set<std::string> &&changed_files) {
    (void)changed_files;
    // TODO: only recompile affected page
    // auto diff a rebuild_pages();
    // ws_send_string("{\"changed_pages\": [\"/\"]}");
    build_all_pages();
    ws_send_string("{\"action\":\"refresh\"}");
  }

  Timeouter timeout{exec};

  void handle_event(Event e) override {
    if ((e.event->mask & (IN_CREATE | IN_DELETE | IN_CLOSE_WRITE)) == 0)
      return;
    timeout.set_timeout(get_filename(e));
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
