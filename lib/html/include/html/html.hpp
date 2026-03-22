#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
namespace html {

using Node = std::variant<struct Element, std::string>;
using Fragment = std::vector<Node>;

std::string node_str(Node const &n);
std::string node_str(Fragment const &n);
std::string node_str(struct Document const &d);

struct Doctype {};

struct Element {
  std::string name;
  bool self_close;
  std::map<std::string, std::optional<std::string>> attrs;
  std::map<std::string, std::string> styles;
  std::vector<Node> children;

  bool has_attribute(std::string const &name) const;
  void set_attribute(std::string const &name, std::optional<std::string> value);
  void set_style_prop(std::string const &name, std::string const &value);
  void remove_attribute(std::string const &name);
};

struct Document {
  std::optional<Doctype> doctype;
  Element root_node;
};

struct AttrForInsertion {
  std::string name;
  std::optional<std::string> value = std::nullopt;
};

using Child = std::variant<Node, Fragment>;

Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<Child> &&children);
Element html(std::string const &tag,
             std::vector<AttrForInsertion> const &attrs); // self closing
Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<AttrForInsertion> &&attrs_spread,
             std::vector<Child> &&children);
Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<AttrForInsertion> &&attrs_spread); // self closing
Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs);
Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<Child> &&children);
Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<AttrForInsertion> &&attrs_spread);
Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<AttrForInsertion> &&attrs_spread,
                   std::vector<Child> &&children);

} // namespace html
