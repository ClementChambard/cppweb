#pragma once

#include "event.hpp"
#include "watch.hpp"
#include <vector>

struct Watcher {
  int fd;
  std::vector<Watch *> watches;
  static bool PRINTALL;

  Watcher();
  virtual ~Watcher();
  std::string get_filename(Event event) const;
  void add_watch_for_path(std::string const &path);
  void remove_watch_for_deletion(Event event);
  void remove_watch(Watch *w);
  Watch *get_watch(std::string const &path);
  Watch *get_watch(int wd);
  void add_watch_for_creation(Event event);
  virtual void handle_event(Event) {}

  static void handle_events();
};
