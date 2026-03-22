#include <html/macros.hpp>
#include <variant>

START_COMPONENT(Button)

DEF_PROP(variant, STRING, true)
DEF_PROP(size, STRING, true)
DEF_PROP(asChild, BOOLEAN, true)

DEF_INTERFACE(Button, true, true)

DEF_BUILDFN(Button, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  std::string variant = params.take_a<std::string>("variant", "primary");
  std::string size = params.take_a<std::string>("size", "default");
  auto asChild = params.take_a<bool>("asChild", false);
  if (class_names.size() != 0)
    class_names += ' ';

  // classes
  class_names += "c-button";
  if (variant == "primary") {
    class_names += " v-primary";
  } else if (variant == "secondary") {
    class_names += " v-secondary";
  } else if (variant == "tertiary") {
    class_names += " v-tertiary";
  } else if (variant == "destructive") {
    class_names += " v-destructive";
  } else if (variant == "outline") {
    class_names += " v-outline";
  } else if (variant == "ghost") {
    class_names += " v-ghost";
  } else if (variant == "muted") {
    class_names += " v-muted";
  } else {
    // error ?
  }

  if (size == "default") {
    class_names += " s-no";
  } else if (size == "sm") {
    class_names += " s-sm";
  } else if (size == "xs") {
    class_names += " s-xs";
  } else if (size == "lg") {
    class_names += " s-lg";
  } else if (size == "icon") {
    class_names += " s-icon";
  }

  auto remainder = params.gather_remaining_attrs();

  if (asChild && params.inner_html.size() == 1 &&
      std::holds_alternative<html::Element>(params.inner_html[0])) {
    auto &res = std::get<html::Element>(params.inner_html[0]);
    res.attrs.insert_or_assign("class", class_names);
    for (auto r : remainder) {
      res.attrs.insert_or_assign(r.name, r.value);
    }
    return {res};
  }

  // clang-format off
  return {
  html::html("button", {
      {"class", class_names}, }, std::move(remainder), 
      {params.inner_html })
  };
  // clang-format on
}

END_COMPONENT(Button)
