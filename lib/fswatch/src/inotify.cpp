#include "watch.hpp"
#include "watcher.hpp"
#include <cstdio>
#include <cstdlib>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

static int INOTIFY_FD = 0;
static int INOTIFY_REFCOUNT = 0;
static std::vector<Watch *> watches;

void in_add_watch(Watch *w) { watches.push_back(w); }
void in_remove_watch(Watch *w) { 
  [[maybe_unused]] auto _ = std::remove_if(watches.begin(), watches.end(), [w](Watch *wa){ return wa == w; });
}

int get_inotify() {
  if (INOTIFY_REFCOUNT <= 0) {
    INOTIFY_FD = inotify_init1(IN_NONBLOCK);
    if (INOTIFY_FD == -1) {
      perror("inotify_init1");
      exit(EXIT_FAILURE);
    }
    INOTIFY_REFCOUNT = 1;
    return INOTIFY_FD;
  }
  INOTIFY_REFCOUNT++;
  return INOTIFY_FD;
}

void release_inotify() {
  INOTIFY_REFCOUNT--;
  if (INOTIFY_REFCOUNT == 0) {
    close(INOTIFY_FD);
  }
}

void in_handle_events() {
  /* Some systems cannot read integer variables if they are not
     properly aligned.  On other systems, incorrect alignment may
     decrease performance.  Hence, the buffer used for reading from
     the inotify file descriptor should have the same alignment as
     struct inotify_event.  */

  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t size;

  /* Loop while events can be read from inotify file descriptor.  */

  for (;;) {

    /* Read some events.  */

    size = read(INOTIFY_FD, buf, sizeof(buf));
    if (size == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /* If the nonblocking read() found no events to read, then
       it returns -1 with errno set to EAGAIN.  In that case,
       we exit the loop.  */

    if (size <= 0)
      break;

    /* Loop over all events in the buffer.  */

    for (char *ptr = buf; ptr < buf + size;
         ptr += sizeof(struct inotify_event) + event->len) {

      event = (const struct inotify_event *)ptr;

      for (auto *w : watches) {
        if (w->wd == event->wd) {
          w->watcher->handle_event({
              .w = w,
              .event = event
          });
          break;
        }
      }
    }
  }
}
