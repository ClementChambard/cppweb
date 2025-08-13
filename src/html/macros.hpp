#ifndef IG_HTML_MACROS_HPP
#define IG_HTML_MACROS_HPP

////// PROTOTYPE HTML MACROS

#include "html/code_builder.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct html_test {
  html_test(std::string n) : name(n) {}
  void add_child(html_test t) { children.push_back(t); }

  std::vector<html_test> children = {};

  void param(std::string k, int value) { params[k] = std::to_string(value); }
  void param(std::string k, std::string value) { params[k] = value; }

  int i = 0;
  std::string name;
  std::unordered_map<std::string, std::string> params = {};

  std::string build() {
    auto builder = html::CodeBuilder(name.c_str());
    for (auto [k, v] : params) {
      builder.placeholder(k.c_str(), v.c_str());
    }
    std::string children_str = "";
    for (auto c : children) {
      children_str += c.build();
    }
    builder.placeholder("children", children_str.c_str());
    return builder.build();
  }
};

#define DECLARE_HTML(var, name)                                                \
  std::string var = "";                                                        \
  for (auto __builder = html_test(name); __builder.i < 1;                      \
       __builder.i++, var = __builder.build())

#define CHILD(name)                                                            \
  for (auto &__parent_builder = __builder, __builder = html_test(name);        \
       __builder.i < 1; __builder.i++, __parent_builder.add_child(__builder))

#define PARAM(key, value) __builder.param(key, value)

#define PARAM_OPT(key, boolean)                                                \
  if (boolean)                                                                 \
  PARAM(key, 1)

#define ADD_HTML(txt)                                                          \
  CHILD("components::value_only") { PARAM("value", txt); }

#endif // !IG_HTML_MACROS_HPP
