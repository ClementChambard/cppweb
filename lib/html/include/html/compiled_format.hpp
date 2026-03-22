#pragma once

#include <map>
#include <string>

namespace html {

std::string exec_compiled_format(std::string const &data);
std::string exec_component(std::string const &name,
                           std::map<std::string, std::string> const &attrs,
                           std::string const &inner_html);

} // namespace html
