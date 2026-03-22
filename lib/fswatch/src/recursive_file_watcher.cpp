#include <recursive_file_watcher.hpp>
#include <filesystem>
#include <sys/inotify.h>

RecursiveFileWatcher::RecursiveFileWatcher(std::string const &root_dir) {
  add_watch_for_path(root_dir);

  for (const auto &dirEntry :
        std::filesystem::recursive_directory_iterator(root_dir)) {
    if (!dirEntry.is_directory())
      continue;
    add_watch_for_path(dirEntry.path());
  }
}

void RecursiveFileWatcher::handle_event(Event event) {
  if (event.event->mask & IN_ISDIR) {
    if (event.event->mask & IN_DELETE) {
      remove_watch_for_deletion(event);
    }
    if (event.event->mask & IN_CREATE) {
      add_watch_for_creation(event);
    }
    if (event.event->mask & IN_MOVED_FROM) {
      remove_watch_for_deletion(event);
    }
    if (event.event->mask & IN_MOVED_TO) {
      add_watch_for_creation(event);
    }
  }
}
