#include <html/macros.hpp>

START_COMPONENT(SVGIcon)
DEF_PROP(class, STRING, true)
DEF_PROP(style, STRING, true)
DEF_PROP(href, STRING, false)
DEF_INTERFACE(SVGIcon, false, false)
DEF_BUILDFN(SVGIcon, params) {
  return {html::html("object",
                     {{"class", params.take_a<std::string>("class")},
                      {"style", params.take_a<std::string>("style")},
                      {"data", params.take_a<std::string>("href")},
                      {"type", "image/svg+xml"}},
                     {}, {})};
}
END_COMPONENT(SVGIcon)
