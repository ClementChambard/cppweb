#ifndef IG_HTML_COMPONENTS_HPP
#define IG_HTML_COMPONENTS_HPP

#include <string>
#include <unordered_map>

namespace html {

std::string
component_string(char const *component_name,
                 std::unordered_map<std::string, std::string> const &params);

} // namespace html

#endif // !IG_HTML_COMPONENTS_HPP
