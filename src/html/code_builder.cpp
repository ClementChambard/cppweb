#include "code_builder.hpp"
#include "components.hpp"
#include <defines.hpp>

namespace html {

std::string CodeBuilder::build() {
  return component_string(this->html_name.c_str(), placeholder_map);
}

} // namespace html
