#include "../commands.hpp"
#include "sys/subprocess.hpp"

void kill() { sys::exe_run("kill_sock", {}, false); }
