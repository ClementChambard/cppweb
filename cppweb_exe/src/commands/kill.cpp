#include "../commands.hpp"
#include "sys/subprocess.hpp"

void kill() {
  sys::exe_run("kill_sock", {"8081"});
  sys::exe_run("kill_sock", {"1234"});
  sys::exe_run("kill_sock", {}, false);
}
