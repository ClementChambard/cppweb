#include <html/macros.hpp>

START_COMPONENT(DropdownMenu)
DEF_PROP(dir, STRING, true) // ltr / rtl
DEF_PROP(open, BOOLEAN, true)
DEF_PROP(defaultOpen, BOOLEAN, true)
DEF_PROP(onOpenChange, STRING, true) // function
DEF_PROP(modal, BOOLEAN, true)
DEF_INTERFACE(DropdownMenu, true, false)
DEF_BUILDFN(DropdownMenu, params) {
  return {html::component("DropdownMenuPrimitive.Root",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenu)

// ===================================================================================================

START_COMPONENT(DropdownMenuTrigger)
DEF_PROP(asChild, BOOLEAN, true)
DEF_INTERFACE(DropdownMenuTrigger, true, true) // same props as button
DEF_BUILDFN(DropdownMenuTrigger, params) {
  return {html::component("DropdownMenuPrimitive.Trigger",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenuTrigger)

// ===================================================================================================

START_COMPONENT(DropdownMenuGroup)
DEF_INTERFACE(DropdownMenuGroup, true, true)
DEF_BUILDFN(DropdownMenuGroup, params) {
  return {html::component("DropdownMenuPrimitive.Group",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenuGroup)

// ===================================================================================================

START_COMPONENT(DropdownMenuPortal)
DEF_INTERFACE(DropdownMenuPortal, true, true)
DEF_BUILDFN(DropdownMenuPortal, params) {
  return {html::component("DropdownMenuPrimitive.Portal",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenuPortal)

// ===================================================================================================

START_COMPONENT(DropdownMenuSub)
DEF_INTERFACE(DropdownMenuSub, true, true)
DEF_BUILDFN(DropdownMenuSub, params) {
  return {html::component("DropdownMenuPrimitive.Sub",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenuSub)

// ===================================================================================================

START_COMPONENT(DropdownMenuRadioGroup)
DEF_INTERFACE(DropdownMenuRadioGroup, true, true)
DEF_BUILDFN(DropdownMenuRadioGroup, params) {
  return {html::component("DropdownMenuPrimitive.RadioGroup",
                          params.gather_remaining_attrs(),
                          {params.inner_html})};
}
END_COMPONENT(DropdownMenuRadioGroup)

// ===================================================================================================

START_COMPONENT(DropdownMenuSubTrigger)

DEF_PROP(inset, BOOLEAN, true)
DEF_INTERFACE(DropdownMenuSubTrigger, true, true)

DEF_BUILDFN(DropdownMenuSubTrigger, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  bool inset = params.take_a<bool>("inset", false);
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "flex cursor-default select-none items-center gap-2 rounded-sm px-2 "
      "py-1.5 text-sm outline-none focus:bg-accent data-[state=open]:bg-accent "
      "[&_svg]:pointer-events-none [&_svg]:size-4 [&_svg]:shrink-0";
  if (inset)
    class_names += " pl-8";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.SubTrigger", {{"class", class_names}}, params.gather_remaining_attrs(), {
      params.inner_html,
      html::component("SVGIcon", {{"href", "public/chevron-right.svg"}, {"class", "ml-auto"}})
    })
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuSubTrigger)

// ===================================================================================================

START_COMPONENT(DropdownMenuSubContent)

DEF_INTERFACE(DropdownMenuSubContent, false, true)

DEF_BUILDFN(DropdownMenuSubContent, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "z-50 min-w-[8rem] overflow-hidden rounded-md border bg-popover p-1 "
      "text-popover-foreground shadow-lg data-[state=open]:animate-in "
      "data-[state=closed]:animate-out data-[state=closed]:fade-out-0 "
      "data-[state=open]:fade-in-0 data-[state=closed]:zoom-out-95 "
      "data-[state=open]:zoom-in-95 data-[side=bottom]:slide-in-from-top-2 "
      "data-[side=left]:slide-in-from-right-2 "
      "data-[side=right]:slide-in-from-left-2 "
      "data-[side=top]:slide-in-from-bottom-2 "
      "origin-[--radix-dropdown-menu-content-transform-origin]";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.SubContent", {{"class", class_names}}, params.gather_remaining_attrs())
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuSubContent)

// ===================================================================================================

START_COMPONENT(DropdownMenuContent)

DEF_PROP(sideOffset, NUMBER, true)
DEF_INTERFACE(DropdownMenuContent, false, true)

DEF_BUILDFN(DropdownMenuContent, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  bool sideOffset = params.take_a<f64>("sideOffset", 4);
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "z-50 max-h-[var(--radix-dropdown-menu-content-available-height)] "
      "min-w-[8rem] overflow-y-auto overflow-x-hidden rounded-md border "
      "bg-popover p-1 text-popover-foreground shadow-md "
      "data-[state=open]:animate-in data-[state=closed]:animate-out "
      "data-[state=closed]:fade-out-0 data-[state=open]:fade-in-0 "
      "data-[state=closed]:zoom-out-95 data-[state=open]:zoom-in-95 "
      "data-[side=bottom]:slide-in-from-top-2 "
      "data-[side=left]:slide-in-from-right-2 "
      "data-[side=right]:slide-in-from-left-2 "
      "data-[side=top]:slide-in-from-bottom-2 "
      "origin-[--radix-dropdown-menu-content-transform-origin]";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.Portal", {}, {
      html::component("DropdownMenuPrimitive.Content", {{"class", class_names}, {"sideOffset", std::to_string(sideOffset)}}, params.gather_remaining_attrs())
    })
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuContent)

// ===================================================================================================

START_COMPONENT(DropdownMenuItem)

DEF_PROP(inset, BOOLEAN, true)
DEF_INTERFACE(DropdownMenuItem, false, true)

DEF_BUILDFN(DropdownMenuItem, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  bool inset = params.take_a<bool>("inset", false);
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "relative flex cursor-default select-none items-center gap-2 rounded-sm "
      "px-2 py-1.5 text-sm outline-none transition-colors focus:bg-accent "
      "focus:text-accent-foreground data-[disabled]:pointer-events-none "
      "data-[disabled]:opacity-50 [&>svg]:size-4 [&>svg]:shrink-0";
  if (inset)
    class_names += " pl-8";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.Item", {{"class", class_names}}, params.gather_remaining_attrs())
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuItem)

// ===================================================================================================

START_COMPONENT(DropdownMenuCheckboxItem)

DEF_PROP(checked, BOOLEAN, true)
DEF_INTERFACE(DropdownMenuCheckboxItem, true, true)

DEF_BUILDFN(DropdownMenuCheckboxItem, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  bool checked = params.take_a<bool>("checked", false);
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "relative flex cursor-default select-none items-center rounded-sm py-1.5 "
      "pl-8 pr-2 text-sm outline-none transition-colors focus:bg-accent "
      "focus:text-accent-foreground data-[disabled]:pointer-events-none "
      "data-[disabled]:opacity-50";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.CheckboxItem", {{"class", class_names}, {"checked", std::to_string(checked)}}, params.gather_remaining_attrs(), {
    html::html("span", {{"class", "absolute left-2 flex h-3.5 w-3.5 items-center justify-center"}}, {
          html::component("DropdownMenuPrimitive.ItemIndicator", {}, {
              html::component("SVGIcon", {{"href", "public/check.svg"}, {"class", "h-4 w-4"}})
              })
        }),
        params.inner_html
      })
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuCheckboxItem)

// ===================================================================================================

START_COMPONENT(DropdownMenuRadioItem)

DEF_INTERFACE(DropdownMenuRadioItem, true, true)

DEF_BUILDFN(DropdownMenuRadioItem, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  if (class_names.size() > 0)
    class_names += ' ';
  class_names +=
      "relative flex cursor-default select-none items-center rounded-sm py-1.5 "
      "pl-8 pr-2 text-sm outline-none transition-colors focus:bg-accent "
      "focus:text-accent-foreground data-[disabled]:pointer-events-none "
      "data-[disabled]:opacity-50";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.RadioItem", {{"class", class_names}}, params.gather_remaining_attrs(), {
    html::html("span", {{"class", "absolute left-2 flex h-3.5 w-3.5 items-center justify-center"}}, {
          html::component("DropdownMenuPrimitive.ItemIndicator", {}, {
              html::component("SVGIcon", {{"href", "public/circle.svg"}, {"class", "h-2 w-2 fill-current"}})
              })
        }),
        params.inner_html
      })
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuRadioItem)

// ===================================================================================================

START_COMPONENT(DropdownMenuLabel)

DEF_PROP(inset, BOOLEAN, true)
DEF_INTERFACE(DropdownMenuLabel, false, true)

DEF_BUILDFN(DropdownMenuLabel, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  bool inset = params.take_a<bool>("inset", false);
  if (class_names.size() > 0)
    class_names += ' ';
  class_names += "px-2 py-1.5 text-sm font-semibold";
  if (inset)
    class_names += " pl-8";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.Label", {
      {"class", class_names}
      }, params.gather_remaining_attrs())
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuLabel)

// ===================================================================================================

START_COMPONENT(DropdownMenuSeparator)

DEF_INTERFACE(DropdownMenuSeparator, false, true)

DEF_BUILDFN(DropdownMenuSeparator, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  if (class_names.size() > 0)
    class_names += ' ';
  class_names += "-mx-1 my-1 h-px bg-muted";

  // clang-format off
  return {
    html::component("DropdownMenuPrimitive.Separator", {
      {"class", class_names}
      }, params.gather_remaining_attrs())
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuSeparator)

// ===================================================================================================

START_COMPONENT(DropdownMenuShortcut)

DEF_INTERFACE(DropdownMenuShortcut, false, true)

DEF_BUILDFN(DropdownMenuShortcut, params) {
  std::string class_names = params.take_a<std::string>("class", "");
  if (class_names.size() > 0)
    class_names += ' ';
  class_names += "ml-auto text-xs tracking-widest opacity-60";

  // clang-format off
  return {
  html::html("span", {
      {"class", class_names}
      }, params.gather_remaining_attrs(), {})
  };
  // clang-format on
}

END_COMPONENT(DropdownMenuShortcut)
