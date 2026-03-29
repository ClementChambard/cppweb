#include <cassert>
#include <cctype>
#include <components.hpp>
#include <html.hpp>
#include <map>
#include <string>
#include <vector>

components::Map *REGISTERED_COMPONENTS;
components::ServerMap *REGISTERED_SERVER_COMPONENTS;

namespace components {

std::optional<Params::Val> Params::take(char const *name) {
  auto it = attrs.find(name);
  if (it == attrs.end())
    return std::nullopt;
  Val out = it->second;
  attrs.erase(it);
  return out;
}

std::vector<html::AttrForInsertion> Params::gather_remaining_attrs() {
  std::vector<html::AttrForInsertion> out{};
  for (auto &[k, v] : attrs) {
    std::optional<std::string> vv = std::nullopt;

    if (v.kind == Interface::Type::NUMBER) {
      vv = std::to_string(v.n);
    } else if (v.kind == Interface::Type::STRING) {
      vv = v.s;
    } else if (v.kind == Interface::Type::BOOLEAN) {
      vv = std::to_string(v.b);
    } else if (v.kind == Interface::Type::NO_VALUE) {
      // do nothing
    } else {
      // TODO: error ?
    }
    out.push_back({k, vv});
  }
  return out;
}

std::vector<std::string> Interface::get_required_props() {
  std::vector<std::string> out;
  for (u32 i = 0; i < prop_count; i++) {
    if (!props[i].optional) {
      out.push_back(props[i].name);
    }
  }
  return out;
}

static std::string strip(std::string s) {
  while (s.size() > 0 && std::isspace(s[0])) {
    s = s.substr(1);
  }
  while (s.size() > 0 && std::isspace(s[s.size() - 1])) {
    s = s.substr(0, s.size() - 1);
  }
  return s;
}

static bool parse_num(std::string const &v, f64 &out) {
  out = 0.0;
  for (auto c : v) {
    if ((c >= '0' && c <= '9') || c == '.')
      continue;
    return false;
  }
  out = std::stod(v);
  return true;
}

static bool try_convert(std::string v, Interface::AttributeValue &out) {
  if (out.kind == Interface::Type::STRING) {
    out.s = v;
    return true;
  }
  v = strip(v);
  f64 num = 0.0;
  bool is_num = parse_num(v, num);
  if (out.kind == Interface::Type::BOOLEAN) {
    if (v == "true" || (is_num && num != 0)) {
      out.b = true;
      return true;
    } else if (v == "false" || (is_num && num == 0)) {
      out.b = false;
      return true;
    }
    return false;
  } else if (out.kind == Interface::Type::NUMBER) {
    if (!is_num)
      return false;
    out.n = num;
    return true;
  } else {
    return false;
  }
}

bool Interface::check_val(std::string k, std::optional<std::string> v,
                          AttributeValue &out) {
  Prop const *p = nullptr;
  for (u32 i = 0; i < prop_count; i++) {
    if (props[i].name == k) {
      p = &props[i];
      break;
    }
  }
  if (!p) {
    if (accepts_more_props) {
      if (v) {
        out.kind = Type::STRING;
        out.s = *v;
      } else {
        out.kind = Type::NO_VALUE;
      }
      return true;
    } else {
      return false;
    }
  }
  out.kind = p->type;
  if (v) {
    return try_convert(*v, out);
  } else {
    if (p->type == Type::NO_VALUE)
      return true;
    if (p->type == Type::BOOLEAN) {
      out.b = true;
      return true;
    }
    return false;
  }
}

Component const *Component::find(std::string const &name) {
  assert(REGISTERED_COMPONENTS != nullptr);
  auto it = REGISTERED_COMPONENTS->find(name);
  if (it == REGISTERED_COMPONENTS->end())
    return nullptr;
  return it->second;
}

ServerComponent const *ServerComponent::find(std::string const &name) {
  assert(REGISTERED_SERVER_COMPONENTS != nullptr);
  auto it = REGISTERED_SERVER_COMPONENTS->find(name);
  if (it == REGISTERED_SERVER_COMPONENTS->end())
    return nullptr;
  return it->second;
}

static constexpr std::string INVALID_COMPONENT_NAME = "INVALID";

std::string const &get_my_name(ServerComponent const *sc) {
  for (auto &[k, v] : *REGISTERED_SERVER_COMPONENTS) {
    if (v == sc)
      return k;
  }
  return INVALID_COMPONENT_NAME;
}

html::Fragment apply_to(html::Element &&e) {
  Component const *c = Component::find(e.name);
  if (c == nullptr) {
    return {"unknown component"};
  }
  auto interface = c->get_interface();
  if (e.self_close != !interface->can_have_children) {
    return {"component error (children)"};
  }

  std::map<std::string, bool> required_check;
  for (auto const &n : interface->get_required_props()) {
    required_check.insert(std::make_pair(n, false));
  }

  Params params;
  params.inner_html = e.children;
  for (auto const &[k, v] : e.attrs) {
    Params::Val new_val;
    bool check = interface->check_val(k, v, new_val);
    if (!check)
      return {"component error (attr)"};
    params.attrs.insert(std::make_pair(k, new_val));
    if (required_check.find(k) != required_check.end()) {
      required_check[k] = true;
    }
  }

  for (auto const &[k, v] : required_check) {
    if (v == false) {
      return {"component error (missing attr)"};
    }
  }

  return c->build_html(std::move(params));
}

html::Fragment ServerComponent::build_html(Params &&params) const {
  auto const &name = get_my_name(this);

  return {html::html('{' + name + '}', params.gather_remaining_attrs(),
                     {params.inner_html})};
}

} // namespace components

static bool map_initialized() {
  return REGISTERED_COMPONENTS != nullptr &&
         REGISTERED_SERVER_COMPONENTS != nullptr;
}

void initialize_maps() {
  REGISTERED_COMPONENTS = new components::Map{};
  REGISTERED_SERVER_COMPONENTS = new components::ServerMap{};
}

void register_component(char const *name, components::Component *c) {
  if (!map_initialized()) {
    initialize_maps();
  }
  REGISTERED_COMPONENTS->insert(std::make_pair(name, c));
}

void register_server_component(char const *name,
                               components::ServerComponent *c) {
  if (!map_initialized()) {
    initialize_maps();
  }
  REGISTERED_SERVER_COMPONENTS->insert(std::make_pair(name, c));
}
