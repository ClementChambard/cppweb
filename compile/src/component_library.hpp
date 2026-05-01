#pragma once

#include <cppweb_static_component.hpp>
#include <vector>

struct ComponentPageData {
  std::string page_title;
  std::vector<std::string> css_to_include;
  std::vector<std::string> js_to_include;

  void reset() {
    page_title = "";
    css_to_include.clear();
    js_to_include.clear();
  }
};

extern ComponentPageData CUR_PAGE_DATA;

struct ComponentLibrary {
  ComponentLibrary(std::string const &so_name);
  ~ComponentLibrary();

  static CPPWEB_StaticComponentDecl const *find(char const *name);

private:
  void *so;
};
