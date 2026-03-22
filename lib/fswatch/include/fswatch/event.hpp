#pragma once

struct Event {
  struct Watch *w;
  // TODO: better
  struct inotify_event const *event;
};
