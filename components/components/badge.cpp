#include <html/macros.hpp>

START_COMPONENT(Badge)

DEF_PROP(variant, STRING, true)
DEF_INTERFACE(Badge, true, true)

DEF_BUILDFN(Badge, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  std::string variant = params.take_a<std::string>("variant", "primary");
  if (class_names.size() != 0)
    class_names += ' ';

  // classes
  class_names += "c-badge";
  if (variant == "primary") {
    class_names += " v-primary";
  } else if (variant == "secondary") {
    class_names += " v-secondary";
  } else if (variant == "destructive") {
    class_names += " v-destructive";
  } else if (variant == "outline") {
    class_names += " v-outline";
  } else {
    // error ?
  }

  // clang-format off
  return {
  html::html("div", {
      {"class", class_names}, },
      params.gather_remaining_attrs(), 
      {params.inner_html })
  };
  // clang-format on
}

END_COMPONENT(Badge)
