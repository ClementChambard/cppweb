#include <cassert>
#include <components.hpp>
#include <html.hpp>
#include <sstream>
#include <variant>

namespace html {

bool Element::has_attribute(std::string const &name) const {
  if (name == "style")
    return styles.size() > 0;
  return attrs.find(name) != attrs.end();
}

std::string fmt_style(std::map<std::string, std::string> const &styles) {
  std::ostringstream oss;
  for (auto const &[k, v] : styles) {
    oss << k << ':' << v << ';';
  }
  return oss.str();
}

std::map<std::string, std::string> unfmt_style(std::string const &value) {
  std::map<std::string, std::string> out;
  u64 pos = 0;
  while (pos < value.size()) {
    u64 name_start_pos = pos;
    u64 name_end_pos = value.find(':', name_start_pos);
    if (pos == std::string::npos)
      break;
    u64 val_start_pos = name_end_pos + 1;
    u64 val_end_pos = value.find(';', val_start_pos);
    out.insert(std::make_pair(
        value.substr(name_start_pos, name_end_pos - name_start_pos),
        value.substr(val_start_pos, val_end_pos - val_start_pos)));
    if (val_end_pos == std::string::npos)
      break;
    pos = val_end_pos + 1;
  }

  return out;
}

void Element::set_attribute(std::string const &name,
                            std::optional<std::string> value) {
  if (name == "style") {
    if (value != std::nullopt) {
      styles = unfmt_style(*value);
    } else {
      styles.clear();
    }
  } else {
    attrs.insert_or_assign(name, value);
  }
}

void Element::set_style_prop(std::string const &name,
                             std::string const &value) {
  styles.insert_or_assign(name, value);
}

void Element::remove_attribute(std::string const &name) { attrs.erase(name); }

std::string escape_value(std::string const &v) {
  // TODO:
  return v;
}

std::string node_str(struct Document const &d) {
  std::string out = "";
  if (d.doctype)
    out += "<!DOCTYPE html>";
  return out + node_str(d.root_node);
}

std::string node_str(Fragment const &n) {
  std::ostringstream oss;
  for (auto const &c : n)
    oss << node_str(c);
  return oss.str();
}

std::string node_str(Node const &n) {
  if (std::holds_alternative<std::string>(n)) {
    return std::get<std::string>(n);
  }
  Element const &e = std::get<Element>(n);

  std::ostringstream oss;
  oss << '<' << e.name;
  for (auto const &[k, v] : e.attrs) {
    oss << ' ';
    oss << k;
    if (v != std::nullopt) {
      oss << "=\"" << escape_value(*v) << '"';
    }
  }
  if (e.styles.size() > 0) {
    oss << " style=\"" << fmt_style(e.styles) << '"';
  }
  if (e.self_close) {
    oss << "/>";
    return oss.str();
  }
  oss << '>';
  for (auto const &c : e.children) {
    oss << node_str(c);
  }
  oss << "</" << e.name << '>';
  return oss.str();
}

Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<Child> &&children) {
  Element e;
  e.name = tag;
  for (auto &&c : children) {
    if (std::holds_alternative<Fragment>(c)) {
      for (auto const &cc : std::get<Fragment>(c)) {
        e.children.push_back(cc);
      }
    } else {
      assert(std::holds_alternative<Node>(c));
      e.children.push_back(std::get<Node>(c));
    }
  }
  e.self_close = false;
  for (auto const &[k, v] : attrs) {
    e.set_attribute(k, v);
  }
  return e;
}

Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<AttrForInsertion> &&attrs_spread,
             std::vector<Child> &&children) {
  Element e;
  e.name = tag;
  for (auto &&c : children) {
    if (std::holds_alternative<Fragment>(c)) {
      for (auto const &cc : std::get<Fragment>(c)) {
        e.children.push_back(cc);
      }
    } else {
      assert(std::holds_alternative<Node>(c));
      e.children.push_back(std::get<Node>(c));
    }
  }
  e.self_close = false;
  for (auto const &[k, v] : attrs) {
    e.set_attribute(k, v);
  }
  for (auto &&[k, v] : attrs_spread) {
    e.set_attribute(k, v);
  }
  return e;
}

Element html(std::string const &tag,
             std::vector<AttrForInsertion> const &attrs) {
  Element e;
  e.name = tag;
  e.children = {};
  e.self_close = true;
  for (auto const &[k, v] : attrs) {
    e.set_attribute(k, v);
  }
  return e;
}

Element html(std::string const &tag, std::vector<AttrForInsertion> const &attrs,
             std::vector<AttrForInsertion> &&attrs_spread) {
  Element e;
  e.name = tag;
  e.children = {};
  e.self_close = true;
  for (auto const &[k, v] : attrs) {
    e.set_attribute(k, v);
  }
  for (auto &&[k, v] : attrs_spread) {
    e.set_attribute(k, v);
  }
  return e;
}

Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   components::Context *ctx) {
  auto h = html(name, attrs);
  h.self_close = true;
  return components::apply_to(std::move(h), ctx);
}

Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<Child> &&children, components::Context *ctx) {
  return components::apply_to(html(name, attrs, std::move(children)), ctx);
}

Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<AttrForInsertion> &&attrs_spread,
                   components::Context *ctx) {
  auto h = html(name, attrs, std::move(attrs_spread));
  h.self_close = true;
  return components::apply_to(std::move(h), ctx);
}

Fragment component(std::string const &name,
                   std::vector<AttrForInsertion> const &attrs,
                   std::vector<AttrForInsertion> &&attrs_spread,
                   std::vector<Child> &&children, components::Context *ctx) {
  return components::apply_to(
      html(name, attrs, std::move(attrs_spread), std::move(children)), ctx);
}

} // namespace html
