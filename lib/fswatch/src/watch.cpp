#include <cstring>
#include <watch.hpp>
#include <watcher.hpp>

#include <sys/inotify.h>

constexpr int WATCH_FLAGS = IN_CLOSE | IN_MOVE | IN_MODIFY | IN_CREATE |
                            IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF;

Watch::Watch(struct Watcher *watcher, std::string const &dir_name)
    : watcher(watcher), dir_name(dir_name) {
  wd = inotify_add_watch(watcher->fd, dir_name.c_str(), WATCH_FLAGS);
  if (wd == -1) {
    fprintf(stderr, "Cannot watch '%s': %s\n", dir_name.c_str(),
            strerror(errno));
    exit(EXIT_FAILURE);
  }
}

Watch::~Watch() { inotify_rm_watch(watcher->fd, wd); }
