#ifndef IG_HTML_MACROS_HPP
#define IG_HTML_MACROS_HPP

////// PROTOTYPE HTML MACROS

#include "html/code_builder.hpp"
#include <string>

struct __build_context {
  __build_context(char const *name, __build_context *parent = nullptr)
      : builder(name), parent(parent) {}
  html::CodeBuilder builder;
  std::string children_str;
  __build_context *parent;
  int i = 0;
  bool cond() const { return i < 1; }
  std::string latch() {
    i++;
    builder.placeholder("children", children_str.c_str());
    if (parent) {
      parent->children_str += builder.build();
      return "";
    } else {
      return builder.build();
    }
  }
};

#define DECLARE_HTML(var, name)                                                \
  std::string var;                                                             \
  for (__build_context __ctx{name}; __ctx.cond(); var = __ctx.latch())

#define CHILD(name)                                                            \
  for (__build_context *p = &__ctx, __ctx{name, p}; __ctx.cond(); __ctx.latch())

#define PARAM(name, value) __ctx.builder.placeholder(name, value)

#define PARAM_OPT(key, boolean)                                                \
  if (boolean)                                                                 \
  PARAM(key, "1")

#define ADD_HTML(txt) __ctx.children_str += txt;

#endif // !IG_HTML_MACROS_HPP
