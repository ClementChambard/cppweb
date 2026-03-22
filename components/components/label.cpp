#include "cn.hpp"

START_COMPONENT(Label)
DEF_INTERFACE(Label, true, true)
DEF_BUILDFN(Label, params) {
  return {html::html(
      "label",
      {{"class",
        cn(params,
           "text-sm font-medium leading-none peer-disabled:cursor-not-allowed "
           "peer-disabled:opacity-70")}},
      params.gather_remaining_attrs(), {params.inner_html})};
}
END_COMPONENT(Label)
