#pragma once

#include <map>
#include <string>

typedef char const *cstr;

namespace cppweb {
struct ServerRenderingContext;
}

struct CPPWEB_ServerComponentDecl {
  cstr name;
  std::string (*render_func)(cppweb::ServerRenderingContext &,
                             std::map<std::string, std::string>, std::string);
};

#define DECLARE_SERVER_COMPONENT(name)                                         \
  std::string name(cppweb::ServerRenderingContext &,                           \
                   std::map<std::string, std::string>, std::string);           \
  extern CPPWEB_ServerComponentDecl CPPWEB_##name##_DECL;

#define SERVER_COMPONENT(cname, ctx, params, inner_html)                       \
  CPPWEB_ServerComponentDecl CPPWEB_##cname##_DECL{.name = #cname,             \
                                                   .render_func = &cname};     \
  std::string cname(cppweb::ServerRenderingContext &ctx,                       \
                    std::map<std::string, std::string> params,                 \
                    std::string inner_html)

#ifndef COMPONENT
#define COMPONENT(name) &CPPWEB_##name##_DECL
#endif

#define TMP_DECLARE_SERVER_COMPONENT(name)                                     \
  namespace components {                                                       \
  extern struct ServerComponent CPPWEB_##name##_DECL;                          \
  }
