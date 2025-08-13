#ifndef IG_HTML_CODE_HPP
#define IG_HTML_CODE_HPP

#include <string>
#include <unordered_map>

namespace html {

struct CodeBuilder {
  CodeBuilder(char const *html_name);
  CodeBuilder &placeholder(char const *placeholder, char const* data);
  std::string build();

  std::string html_name;
  std::unordered_map<std::string, std::string> placeholder_map;
};

} // namespace html

#endif // !IG_HTML_CODE_HPP
