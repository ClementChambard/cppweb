#include "code_instance.hpp"
#include "html/components.hpp"
#include "sys/logger.hpp"
#include <defines.hpp>
#include <sys/read_file.hpp>
#include <unordered_map>

namespace html {

// TODO: cache invalidation

static std::unordered_map<std::string, std::string> LOADED_CODE;

extern std::string minify(std::string_view input);

std::string get_file_name(char const *html_name) {
  std::string html = html_name;
  u64 pos = 0;
  while ((pos = html.find("::", pos)) != std::string::npos) {
    html.replace(pos, 2, "/");
  }
  return "html/" + html + ".html";
}

void load_code(char const *html_name, bool no_componentation) {
  std::string html_file_name = get_file_name(html_name);

  std::string code = sys::read_text_file(html_file_name.c_str());

#ifndef RELEASE
  code = minify(code);
#endif

  LOADED_CODE[html_name] = code;
}

void cleanup_cache() {
  LOADED_CODE.clear();
}

std::string get_code_instance(char const *html_name, bool no_componentation) {
  if (LOADED_CODE.find(html_name) == LOADED_CODE.end()) {
    load_code(html_name, no_componentation);
  }

  return LOADED_CODE[html_name];
}

}
