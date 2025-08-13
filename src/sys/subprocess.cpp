#include "subprocess.hpp"
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace sys {

void subprocess_run(std::span<char const *> args) {
  auto pid = fork();
  if (pid == 0) {
    char const *args_array[64]; // max 64 arg. is this ok ?
    std::memcpy(args_array, args.data(), args.size() * sizeof(args[0]));
    args_array[args.size()] = nullptr;
    execvp(args[0], const_cast<char *const *>(args_array));
  } else {
    waitpid(pid, nullptr, 0);
  }
}

} // namespace sys
