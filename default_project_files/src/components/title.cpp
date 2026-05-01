#include "components.hpp"

STATIC_COMPONENT Title(CPPWEB_StaticComponentBody body, cstr color) {
  return "<h1 class=\"title\" style=\"color:" + std::string(color) + "\">" +
         body.content + "</h1>";
}

CPPWEB_StaticComponentAttrDecl CPPWEB_Title_ATTR_color{
    .name = "color",
    .type = CPPWEB_ATTR_STRING,
    .optional = true,
    .default_value = {.str = "#000000"},
    .validator = nullptr};

CPPWEB_StaticComponentDecl CPPWEB_Title_DECL{
    .name = "Title",
    .attrs = &CPPWEB_Title_ATTR_color,
    .attrs_count = 1,
    .can_have_more_attrs = false,
    .can_have_body = true,
    .render_func = [](CPPWEB_StaticComponentArgs args) {
      return Title(args.body, args.declared_args[0].str);
    }};
