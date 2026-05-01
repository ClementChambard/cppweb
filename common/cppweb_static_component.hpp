#pragma once

#include "defines.hpp"
#include <string>

typedef char const *cstr;

union CPPWEB_Val {
  cstr str;
  long long int number_i;
  double number_f;
};

struct CPPWEB_StaticComponentArgList {
  u32 count;
  cstr *names;
  cstr *values;
};

struct CPPWEB_StaticComponentBody {
  std::string const &content;
};

struct CPPWEB_StaticComponentArgs {
  CPPWEB_Val *declared_args;
  CPPWEB_StaticComponentArgList remaining_args;
  CPPWEB_StaticComponentBody body;
};

enum CPPWEB_StaticComponentAttrType {
  CPPWEB_ATTR_STRING = 1,
  CPPWEB_ATTR_INTEGER = 2,
  CPPWEB_ATTR_FLOAT = 4,
  CPPWEB_ATTR_BOOL = 8,
};

typedef bool (*CPPWEB_StaticComponentAttrValidator)(CPPWEB_Val const &);

struct CPPWEB_StaticComponentAttrDecl {
  cstr name;
  CPPWEB_StaticComponentAttrType type;
  bool optional;
  CPPWEB_Val default_value;
  CPPWEB_StaticComponentAttrValidator validator;
};

struct CPPWEB_StaticComponentDecl {
  cstr name;
  CPPWEB_StaticComponentAttrDecl *attrs;
  u32 attrs_count;
  bool can_have_more_attrs;
  bool can_have_body;
  std::string (*render_func)(CPPWEB_StaticComponentArgs args);
};

#define DECLARE_STATIC_COMPONENT(name, ...)                                    \
  std::string name(__VA_ARGS__);                                               \
  extern CPPWEB_StaticComponentDecl CPPWEB_##name##_DECL;

#define STATIC_COMPONENT std::string

#ifndef COMPONENT
#define COMPONENT(name) &CPPWEB_##name##_DECL
#endif
