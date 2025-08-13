#include "code_builder.hpp"
#include <defines.hpp>
#include "components.hpp"

namespace html {

CodeBuilder::CodeBuilder(char const *html_name) {
  this->html_name = html_name;
}

CodeBuilder &CodeBuilder::placeholder(char const *placeholder, char const* data) {
  placeholder_map[placeholder] = data;
  return *this;
}

std::string CodeBuilder::build() {
  return component_string(this->html_name.c_str(), placeholder_map);
}

}
