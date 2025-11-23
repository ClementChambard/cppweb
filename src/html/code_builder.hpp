#ifndef IG_HTML_CODE_HPP
#define IG_HTML_CODE_HPP

#include <string>
#include <unordered_map>

namespace html {

struct CodeBuilder {
  CodeBuilder(char const *html_name) : html_name(html_name) {}

  template <typename T> CodeBuilder &placeholder(char const *placeholder, T t) {
    return this->placeholder(placeholder, std::to_string(t));
  }
  CodeBuilder &placeholder(char const *placeholder, char const *data) {
    placeholder_map[placeholder] = data;
    return *this;
  }
  CodeBuilder &placeholder(char const *placeholder, std::string const &data) {
    placeholder_map[placeholder] = data;
    return *this;
  }
  std::string build();

  std::string html_name;
  std::unordered_map<std::string, std::string> placeholder_map;
};

} // namespace html

#endif // !IG_HTML_CODE_HPP
