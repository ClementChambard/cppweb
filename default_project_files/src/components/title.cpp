#include <html/macros.hpp>

START_COMPONENT(Title)
DEF_PROP(color, STRING, true)
DEF_INTERFACE(Title, true, false)
DEF_BUILDFN(Title, params) {
  auto color = params.take_a<std::string>("color", "#000000");

  auto elt = html::html("h1", {{"class", "title"}}, {params.inner_html});

  elt.set_style_prop("color", color);

  return {elt};
}
END_COMPONENT(Title)
