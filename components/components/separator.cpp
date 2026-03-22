#include "cn.hpp"

START_COMPONENT(Separator)
DEF_PROP(orientation, STRING, true)
DEF_PROP(decorative, BOOLEAN, true)
DEF_INTERFACE(Separator, false, true)
DEF_BUILDFN(Separator, props) {
  auto orientation = props.take_a<std::string>("orientation", "horizontal");
  bool decorative = props.take_a<bool>("decorative", true);
  if (orientation != "horizontal" && orientation != "vertical")
    orientation = "horizontal";

  auto class_names =
      cn(props, std::string("c-separator ") +
                    (orientation == "horizontal" ? "v-h" : "v-v"));

  std::vector<html::AttrForInsertion> attrs = {
      {"data-orientation", orientation}, {"class", class_names}};

  if (decorative) {
    attrs.push_back({"role", "none"});
  } else {
    attrs.push_back({"role", "separator"});
    if (orientation == "vertical")
      attrs.push_back({"aria-orientation", "vertical"});
  }

  return {html::html("div", attrs, props.gather_remaining_attrs(),
                     {props.inner_html})};
}
END_COMPONENT(Separator)

START_COMPONENT(DottedSeparator)
DEF_PROP(orientation, STRING, true)
DEF_PROP(decorative, BOOLEAN, true)
DEF_PROP(color, STRING, true)
DEF_PROP(size, STRING, true)
DEF_PROP(dotSize, STRING, true)
DEF_PROP(gapSize, STRING, true)
DEF_INTERFACE(DottedSeparator, false, true)
DEF_BUILDFN(DottedSeparator, props) {
  auto orientation = props.take_a<std::string>("orientation", "horizontal");
  auto color = props.take_a<std::string>("color", "#d4d4d8");
  auto size = props.take_a<std::string>("size", "2px");
  auto dotSize = props.take_a<std::string>("dotSize", "2px");
  auto gapSize = props.take_a<std::string>("gapSize", "6px");
  bool decorative = props.take_a<bool>("decorative", true);
  if (orientation != "horizontal" && orientation != "vertical")
    orientation = "horizontal";
  bool isHorizontal = orientation != "vertical";
  if (dotSize.size() > 2 && dotSize[dotSize.size() - 1] == 'x' &&
      dotSize[dotSize.size() - 2] == 'p') {
    dotSize = dotSize.substr(0, dotSize.size() - 2);
  }
  if (gapSize.size() > 2 && gapSize[gapSize.size() - 1] == 'x' &&
      gapSize[gapSize.size() - 2] == 'p') {
    gapSize = gapSize.substr(0, gapSize.size() - 2);
  }

  auto class_names = cn(props, std::string("c-dotted-separator ") +
                                   (isHorizontal ? "v-h" : "v-v"));

  std::vector<html::AttrForInsertion> attrs = {
      {"data-orientation", orientation}, {"class", class_names}};

  if (decorative) {
    attrs.push_back({"role", "none"});
  } else {
    attrs.push_back({"role", "separator"});
    if (!isHorizontal)
      attrs.push_back({"aria-orientation", "vertical"});
  }

  u32 dot_size = std::stoi(dotSize);
  u32 gap_size = std::stoi(gapSize);
  u32 full_size = dot_size + gap_size;
  std::string inner_style = "background-image: radial-gradient(circle, " +
                            color + " 25%, transparent 25%);";
  if (isHorizontal) {
    inner_style += "height: " + size +
                   ";"
                   "background-size: " +
                   std::to_string(full_size) + "px " + size + ";";
  } else {
    inner_style += "width: " + size +
                   ";"
                   "background-size: " +
                   size + " " + std::to_string(full_size) + "px;";
  }

  return {html::html(
      "div", attrs, props.gather_remaining_attrs(),
      {html::html("div", {{"class", "x-inner"}, {"style", inner_style}}, {},
                  {})})};
}
END_COMPONENT(DottedSeparator)
