#include "builtin_components.hpp"
#include "page_data.hpp"

CPPWEB_StaticComponentDecl CPPWEB_ServerComponent_DECL = {
    .name = nullptr,
    .attrs = nullptr,
    .attrs_count = 0,
    .can_have_more_attrs = true,
    .can_have_body = true,
    .render_func = nullptr,
};

static CPPWEB_StaticComponentAttrDecl CPPWEB_src_ATTR = {.name = "src",
                                                         .type =
                                                             CPPWEB_ATTR_STRING,
                                                         .optional = false,
                                                         .default_value = {},
                                                         .validator = nullptr};

static CPPWEB_StaticComponentAttrDecl CPPWEB_val_ATTR = {.name = "val",
                                                         .type =
                                                             CPPWEB_ATTR_STRING,
                                                         .optional = false,
                                                         .default_value = {},
                                                         .validator = nullptr};

CPPWEB_StaticComponentDecl CPPWEB_UseJs_DECL = {
    .name = "UseJs",
    .attrs = &CPPWEB_src_ATTR,
    .attrs_count = 1,
    .can_have_more_attrs = false,
    .can_have_body = false,
    .render_func = [](CPPWEB_StaticComponentArgs args) {
      CUR_PAGE_DATA.js_to_include.push_back(args.declared_args[0].str);
      return std::string();
    }};

CPPWEB_StaticComponentDecl CPPWEB_UseCss_DECL = {
    .name = "UseCss",
    .attrs = &CPPWEB_src_ATTR,
    .attrs_count = 1,
    .can_have_more_attrs = false,
    .can_have_body = false,
    .render_func = [](CPPWEB_StaticComponentArgs args) {
      CUR_PAGE_DATA.css_to_include.push_back(args.declared_args[0].str);
      return std::string();
    }};

CPPWEB_StaticComponentDecl CPPWEB_PageTitle_DECL = {
    .name = "PageTitle",
    .attrs = &CPPWEB_val_ATTR,
    .attrs_count = 1,
    .can_have_more_attrs = false,
    .can_have_body = false,
    .render_func = [](CPPWEB_StaticComponentArgs args) {
      if (CUR_PAGE_DATA.page_title == "")
        CUR_PAGE_DATA.page_title = args.declared_args[0].str;
      return std::string();
    }};

CPPWEB_StaticComponentDecl CPPWEB_Children_DECL = {
    .name = "Children",
    .attrs = nullptr,
    .attrs_count = 0,
    .can_have_more_attrs = false,
    .can_have_body = false,
    .render_func = [](CPPWEB_StaticComponentArgs) {
      return std::string("{{{CHILDREN}}}");
    }};
