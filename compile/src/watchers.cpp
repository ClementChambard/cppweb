#include "watchers.hpp"
#include "config.hpp"
#include <sys/poll.h>

void start_watcher_loop() {
  auto cppwatcher = CppWatcher(CONFIG.cpp_dir);
  auto sowatcher = SoWatcher(CONFIG.cpp_bin);

  /* Prepare for polling.  */

  nfds_t nfds = 2;
  struct pollfd fds[2];
  fds[0].fd = STDIN_FILENO; /* Console input */
  fds[0].events = POLLIN;

  fds[1].fd = cppwatcher.fd; /* Inotify input */
  fds[1].events = POLLIN;

  /* Wait for events and/or terminal input.  */

  while (1) {
    int poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {

      if (fds[0].revents & POLLIN) {

        /* Console input is available.  Empty stdin and quit.  */

        char buf;
        while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
          continue;
        break;
      }

      if (fds[1].revents & POLLIN) {

        /* Inotify events are available.  */

        Watcher::handle_events();
      }
    }
  }
}
