#pragma once

#include "components.hpp"

#define START_COMPONENT(name)                                                  \
  namespace components {                                                       \
  class name : Component {                                                     \
  public:                                                                      \
    name() { register_component(#name, this); }                                \
    Interface *get_interface() const override;                                 \
    html::Fragment build_html(Params &&) const override;                       \
  };                                                                           \
  static Interface::Prop _s_##name##_props[] = {

#define START_SERVER_COMPONENT(name)                                           \
  namespace components {                                                       \
  class name : ServerComponent {                                               \
  public:                                                                      \
    name() {                                                                   \
      register_component(#name, this);                                         \
      register_server_component(#name, this);                                  \
    }                                                                          \
    Interface *get_interface() const override;                                 \
    std::string exec(html::ServerRenderingContext &ctx,                        \
                     std::map<std::string, std::string> params,                \
                     std::string inner_html) const override;                   \
  };                                                                           \
  static Interface::Prop _s_##name##_props[] = {

#define DEF_PROP(name, ty, optional) {#name, Interface::Type::ty, optional},

#define DEF_INTERFACE(name, can_have_children, accepts_more_props)             \
  }                                                                            \
  ;                                                                            \
  static Interface _s_##name##_interface = {                                   \
      can_have_children, accepts_more_props,                                   \
      sizeof(_s_##name##_props) / sizeof(_s_##name##_props[0]),                \
      _s_##name##_props};                                                      \
  Interface *name::get_interface() const { return &_s_##name##_interface; }

#define DEF_BUILDFN(name, params)                                              \
  html::Fragment name::build_html(Params &&params) const

#define DEF_SERVER_EXEC(name, ctx, params, inner_html)                         \
  std::string name::exec(html::ServerRenderingContext &ctx,                    \
                         std::map<std::string, std::string> params,            \
                         std::string inner_html) const

#define END_COMPONENT(name)                                                    \
  static name _s_##name##_instance;                                            \
  }
