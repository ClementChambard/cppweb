#pragma once

#include <string>
struct Watch {
  Watch(struct Watcher *watcher, std::string const &dir_name);
  ~Watch();

  int wd;
  struct Watcher *watcher;
private:
  std::string dir_name;

  friend struct Watcher;
};
