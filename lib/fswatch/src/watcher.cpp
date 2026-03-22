#include "inotify.hpp"
#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>
#include <watcher.hpp>

bool Watcher::PRINTALL = false;

Watcher::Watcher() { fd = get_inotify(); }

Watcher::~Watcher() {
  for (auto w : watches) {
    in_remove_watch(w);
    delete w;
  }
  release_inotify();
}

std::string Watcher::get_filename(Event event) const {
  std::string name = event.event->len ? event.event->name : "";
  return event.w->dir_name + '/' + name;
}

void Watcher::add_watch_for_path(std::string const &path) {
  watches.push_back(new Watch{this, path});
  in_add_watch(watches.back());
  if (PRINTALL)
    std::cout << "Now watching: " << path << '\n';
}

void Watcher::remove_watch_for_deletion(Event event) {
  remove_watch(get_watch(get_filename(event)));
}

void Watcher::remove_watch(Watch *w) {
  auto it = std::find_if(watches.begin(), watches.end(),
                         [w](auto ww) { return ww == w; });
  if (it != watches.end())
    watches.erase(it);
  if (PRINTALL)
    std::cout << "Removed watch " << w->dir_name << '\n';
  in_remove_watch(w);
  delete w;
}

Watch *Watcher::get_watch(std::string const &path) {
  auto it = std::find_if(watches.begin(), watches.end(),
                         [&path](auto w) { return w->dir_name == path; });
  return it != watches.end() ? *it : nullptr;
}

Watch *Watcher::get_watch(int wd) {
  auto it = std::find_if(watches.begin(), watches.end(),
                         [wd](auto w) { return w->wd == wd; });
  return it != watches.end() ? *it : nullptr;
}

void Watcher::add_watch_for_creation(Event event) {
  add_watch_for_path(get_filename(event));
}

void Watcher::handle_events() { in_handle_events(); }
