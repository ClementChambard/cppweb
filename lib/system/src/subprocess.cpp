#include <cstring>
#include <map>
#include <string>
#include <subprocess.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

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

static std::map<std::string, std::string> EXPORTED_ENV;
void export_env_var(std::string_view name, std::string_view value) {
  EXPORTED_ENV.insert(std::make_pair(std::string(name), std::string(value)));
}

int exe_run(std::string const &exe_name, std::vector<std::string> args,
            bool fork, bool wait) {
  // TODO: this is linux only
  pid_t pid = 0;
  if (fork) {
    pid = ::fork();
  }
  if (pid == 0) {
    char const *args_array[64]; // TODO: put more ?
    char const **ptr = &args_array[0];
    *ptr++ = exe_name.c_str();
    for (auto &a : args) {
      *ptr++ = a.c_str();
    }
    *ptr = nullptr;
    if (EXPORTED_ENV.size() == 0) {
      execvp(exe_name.c_str(), const_cast<char *const *>(args_array));
    }
    unsigned int i = 0;
    std::string env_strs[64]; // TODO: put more ?
    char const *env_array[64];
    for (auto const &[k, v] : EXPORTED_ENV) {
      env_strs[i] = k + "=" + v;
      env_array[i] = env_strs[i].c_str();
      i++;
    }
    env_array[i] = nullptr;
    execvpe(exe_name.c_str(), const_cast<char *const *>(args_array),
            const_cast<char *const *>(env_array));
  } else if (wait) {
    waitpid(pid, nullptr, 0);
    return 0;
  }
  return pid;
}

} // namespace sys
