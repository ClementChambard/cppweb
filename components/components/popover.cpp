#include "cn.hpp"
#include <variant>

struct Elts {
  html::Element *trigger = nullptr;
  html::Element *content = nullptr;
  html::Element *anchor = nullptr;
};

Elts resolve_popover_elements(html::Fragment &inner, std::string const &id) {
  Elts out;
  for (auto &c : inner) {
    if (!std::holds_alternative<html::Element>(c))
      continue;
    auto &e = std::get<html::Element>(c);
    if (e.has_attribute("cpp-popover-trigger")) {
      e.remove_attribute("cpp-popover-trigger");
      out.trigger = &e;
    } else if (e.has_attribute("cpp-popover-content")) {
      e.remove_attribute("cpp-popover-content");
      out.content = &e;
    } else if (e.has_attribute("cpp-popover-anchor")) {
      e.remove_attribute("cpp-popover-anchor");
      out.anchor = &e;
    } else if (e.has_attribute("cpp-popover-close")) {
      e.remove_attribute("cpp-popover-close");
      e.set_attribute("popovertarget", id);
    }
    if (e.children.size() > 0) {
      resolve_popover_elements(e.children, id);
    }
  }
  return out;
}

START_COMPONENT(Popover)
DEF_PROP(id, STRING, false)
DEF_INTERFACE(Popover, true, false)
DEF_BUILDFN(Popover, params) {
  auto id = params.take_a<std::string>("id");
  auto inner = params.inner_html;

  auto [trigger, content, anchor] = resolve_popover_elements(inner, id);

  auto anchor_name = "--" + id + "-anchor";

  if (trigger) {
    if (!anchor)
      trigger->set_style_prop("anchor-name", anchor_name);
    trigger->set_attribute("popovertarget", id);
  }
  if (content) {
    content->set_attribute("id", id);
    content->set_style_prop("position-anchor", anchor_name);
  }
  if (anchor) {
    anchor->set_style_prop("anchor-name", anchor_name);
  }

  return {inner};
}
END_COMPONENT(Popover)

START_COMPONENT(PopoverAnchor)
DEF_PROP(asChild, BOOLEAN, true)
DEF_INTERFACE(PopoverAnchor, true, true)
DEF_BUILDFN(PopoverAnchor, params) {
  auto asChild = params.take_a<bool>("asChild", false);

  auto remainder = params.gather_remaining_attrs();

  if (asChild && params.inner_html.size() == 1 &&
      std::holds_alternative<html::Element>(params.inner_html[0])) {
    auto &res = std::get<html::Element>(params.inner_html[0]);
    res.set_attribute("cpp-popover-anchor", "");
    for (auto r : remainder) {
      res.set_attribute(r.name, r.value);
    }
    return {res};
  }

  return {html::html("div",
                     {
                         {"cpp-popover-anchor", ""},
                     },
                     std::move(remainder), {params.inner_html})};
}
END_COMPONENT(PopoverAnchor)

START_COMPONENT(PopoverTrigger)
DEF_PROP(asChild, BOOLEAN, true)
DEF_INTERFACE(PopoverTrigger, true, true)
DEF_BUILDFN(PopoverTrigger, params) {
  auto asChild = params.take_a<bool>("asChild", false);

  auto remainder = params.gather_remaining_attrs();

  if (asChild && params.inner_html.size() == 1 &&
      std::holds_alternative<html::Element>(params.inner_html[0])) {
    auto &res = std::get<html::Element>(params.inner_html[0]);
    res.set_attribute("cpp-popover-trigger", "");
    for (auto r : remainder) {
      res.set_attribute(r.name, r.value);
    }
    return {res};
  }

  return {html::html("button",
                     {
                         {"cpp-popover-trigger", ""},
                     },
                     std::move(remainder), {params.inner_html})};
}
END_COMPONENT(PopoverTrigger)

START_COMPONENT(PopoverContent)
DEF_PROP(side, STRING, true)
DEF_PROP(sideOffset, NUMBER, true)
DEF_INTERFACE(PopoverContent, true, true)
DEF_BUILDFN(PopoverContent, params) {
  auto side = params.take_a<std::string>("side", "bottom");
  auto side_offset = params.take_a<f64>("sideOffset", 0.0);
  std::string opposite_side = side == "bottom" ? "top"
                              : side == "top"  ? "bottom"
                              : side == "left" ? "right"
                                               : "left";

  std::vector<html::AttrForInsertion> attrs;
  attrs.push_back({"cpp-popover-content", ""});
  attrs.push_back({"popover", std::nullopt});
  attrs.push_back({"data-side", side});
  attrs.push_back({"class", cn(params, "c-popover-content")});
  auto elm = html::html("div", attrs, params.gather_remaining_attrs(),
                        {params.inner_html});

  if (side_offset != 0.0) {
    elm.set_style_prop("margin-" + opposite_side,
                       std::to_string(side_offset) + "px");
  }

  return {elm};
}
END_COMPONENT(PopoverContent)

START_COMPONENT(PopoverClose)
DEF_PROP(asChild, BOOLEAN, true)
DEF_INTERFACE(PopoverClose, true, true)
DEF_BUILDFN(PopoverClose, params) {
  auto asChild = params.take_a<bool>("asChild", false);

  auto remainder = params.gather_remaining_attrs();

  if (asChild && params.inner_html.size() == 1 &&
      std::holds_alternative<html::Element>(params.inner_html[0])) {
    auto &res = std::get<html::Element>(params.inner_html[0]);
    res.set_attribute("cpp-popover-close", "");
    for (auto r : remainder) {
      res.set_attribute(r.name, r.value);
    }
    return {res};
  }

  return {html::html("button",
                     {
                         {"cpp-popover-close", ""},
                     },
                     std::move(remainder), {params.inner_html})};
}
END_COMPONENT(PopoverClose)

START_COMPONENT(PopoverArrow)
DEF_PROP(class, STRING, true)
DEF_INTERFACE(PopoverArrow, false, false)
DEF_BUILDFN(PopoverArrow, params) {
  auto class_names = cn(params, "c-popover-arrow");

  // clang-format off
  return {
    html::html("span", {{"style", "position: absolute; top: 0px; transform-origin: center 0px; transform: rotate(180deg); left: 125px;"}}, {}, {
      html::html("svg", {
          {"width", "10"},
          {"height", "5"},
          {"viewBox", "0 0 30 10"},
          {"preserveAspectRatio", "none"},
          {"style", "display:block;"}
          }, {
        html::html("polygon", {{"points", "0,0 30,0 15,10"}}, {}, {})
      })
    })
  };
  // clang-format on
}
END_COMPONENT(PopoverArrow)
