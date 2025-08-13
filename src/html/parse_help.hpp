#ifndef IG_HTML_PARSE_HELP_HPP
#define IG_HTML_PARSE_HELP_HPP

#include <defines.hpp>
#include <string_view>

namespace html {

void skip_spaces(std::string_view &s);
void skip_1(std::string_view &s);
std::string_view read_balanced(std::string_view &s, char const *beg,
                               char const *end);
std::string_view split_at(std::string_view &s, u64 pos, bool include = true);
std::string_view get_until_space_or(std::string_view &s, char const *orelse = "");

} // namespace html

#endif // !IG_HTML_PARSE_HELP_HPP
