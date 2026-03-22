#pragma once

void in_add_watch(struct Watch *w);
void in_remove_watch(struct Watch *w);
void in_handle_events();
int get_inotify();
void release_inotify();
