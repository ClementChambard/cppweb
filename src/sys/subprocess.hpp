#ifndef IG_SYS_SUBPROCESS_HPP
#define IG_SYS_SUBPROCESS_HPP

#include <span>
#include <string>

namespace sys {

void subprocess_run(std::span<char const *> args);

} // namespace sys

#endif // !IG_SYS_SUBPROCESS_HPP
