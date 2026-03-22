#pragma once

#include "cn.hpp"

template <bool has_child = true>
inline html::Element styled_html(std::string const &tag,
                                 components::Params &&params,
                                 std::string base_classes) {
  if constexpr (has_child) {
    return html::html(tag, {{"class", cn(params, base_classes)}},
                      params.gather_remaining_attrs(), {params.inner_html});
  } else {
    return html::html(tag, {{"class", cn(params, base_classes)}},
                      params.gather_remaining_attrs());
  }
}

inline html::Element styled_div(components::Params &&params,
                                std::string base_classes) {
  return styled_html("div", std::move(params), base_classes);
}

#define DEFINE_STYLED_HTML_COMPONENT(name, tag, classes)                       \
  START_COMPONENT(name)                                                        \
  DEF_INTERFACE(name, true, true)                                              \
  DEF_BUILDFN(name, params) {                                                  \
    return {styled_html(tag, std::move(params), classes)};                     \
  }                                                                            \
  END_COMPONENT(name)

#define DEFINE_STYLED_NOCHILD_HTML_COMPONENT(name, tag, classes)               \
  START_COMPONENT(name)                                                        \
  DEF_INTERFACE(name, false, true)                                             \
  DEF_BUILDFN(name, params) {                                                  \
    return {styled_html<false>(tag, std::move(params), classes)};              \
  }                                                                            \
  END_COMPONENT(name)

#define DEFINE_STYLED_DIV_COMPONENT(name, classes)                             \
  DEFINE_STYLED_HTML_COMPONENT(name, "div", classes)
