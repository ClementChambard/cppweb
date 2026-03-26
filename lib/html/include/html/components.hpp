#pragma once

#include "html.hpp"
#include "server_rendering_context.hpp"
#include <defines.hpp>
#include <map>
#include <optional>
#include <string>

namespace components {

struct Interface {
  enum class Type {
    STRING,
    NUMBER,
    BOOLEAN,
    NO_VALUE,
  };
  struct AttributeValue {
    AttributeValue(std::string const &s) : kind(Type::STRING), s(s) {}
    AttributeValue(bool b) : kind(Type::BOOLEAN), b(b) {}
    AttributeValue(f64 n) : kind(Type::NUMBER), n(n) {}
    AttributeValue() : kind(Type::NO_VALUE) {}
    Type kind;
    std::string s{};
    bool b{};
    f64 n{};
    template <typename T> T get() { return T{}; }
  };
  template <typename T>
  static constexpr Interface::Type Typeof = Interface::Type::NO_VALUE;
  struct Prop {
    char const *name;
    Type type;
    bool optional;
  };
  bool can_have_children;
  bool accepts_more_props;
  u32 prop_count;
  Prop const *props;

  std::vector<std::string> get_required_props();
  bool check_val(std::string k, std::optional<std::string> v,
                 AttributeValue &out);
};

template <>
constexpr Interface::Type Interface::Typeof<std::string> =
    Interface::Type::STRING;
template <>
constexpr Interface::Type Interface::Typeof<f64> = Interface::Type::NUMBER;
template <>
constexpr Interface::Type Interface::Typeof<bool> = Interface::Type::BOOLEAN;

struct Params {
  using Val = Interface::AttributeValue;
  std::map<std::string, Val> attrs;
  html::Fragment inner_html;

  void add_param(char const *key, std::string const &val) {
    attrs.insert(std::make_pair<std::string, Val>(key, Val(val)));
  }
  void add_param(char const *key, char const *val) {
    attrs.insert(std::make_pair<std::string, Val>(key, Val(std::string(val))));
  }
  void add_param(char const *key, f64 val) {
    attrs.insert(std::make_pair<std::string, Val>(key, Val(val)));
  }
  void add_param(char const *key, bool val) {
    attrs.insert(std::make_pair<std::string, Val>(key, Val(val)));
  }
  void add_param(char const *key) {
    attrs.insert(std::make_pair<std::string, Val>(key, {}));
  }

  std::optional<Val> take(char const *name);
  template <typename T> T take_a(char const *name, T alt = T{}) {
    auto v = take(name);
    if (v == std::nullopt)
      return alt;
    if (v->kind != Interface::Typeof<T>)
      return alt;
    return v->get<T>();
  }
  bool take_novalprop(char const *name) {
    auto v = take(name);
    if (v == std::nullopt)
      return false;
    return v->kind == Interface::Type::NO_VALUE;
  }
  std::vector<html::AttrForInsertion> gather_remaining_attrs();
};

template <> inline f64 Params::Val::get<f64>() { return n; }
template <> inline bool Params::Val::get<bool>() { return b; }
template <> inline std::string Params::Val::get<std::string>() { return s; }

class Component {
public:
  virtual Interface *get_interface() const = 0;
  virtual html::Fragment build_html(Params &&) const = 0;

  static Component const *find(std::string const &name);
};

class ServerComponent : public Component {
public:
  html::Fragment build_html(Params &&) const override;
  virtual std::string exec(html::ServerRenderingContext &ctx,
                           std::map<std::string, std::string> params,
                           std::string inner_html) const = 0;

  static ServerComponent const *find(std::string const &name);
};

using Map = std::map<std::string, Component const *>;
using ServerMap = std::map<std::string, ServerComponent const *>;

html::Fragment apply_to(html::Element &&e);

} // namespace components

void register_component(char const *name, components::Component *c);
void register_server_component(char const *name,
                               components::ServerComponent *c);

#ifdef HTML_COMPONENTS_IMPL

extern components::Map *REGISTERED_COMPONENTS;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;

extern "C" {

components::Map *get_registered_components() { return REGISTERED_COMPONENTS; }

components::ServerMap *get_registered_server_components() {
  return REGISTERED_SERVER_COMPONENTS;
}
}
#endif
