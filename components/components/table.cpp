#include "styled_html.hpp"

START_COMPONENT(Table)
DEF_INTERFACE(Table, true, true)
DEF_BUILDFN(Table, params) {
  return {html::html("div", {{"class", "c-table"}},
                     {html::html("table", params.gather_remaining_attrs(),
                                 {params.inner_html})})};
}
END_COMPONENT(Table)

DEFINE_STYLED_HTML_COMPONENT(TableHeader, "thead", "c-table-header")
DEFINE_STYLED_HTML_COMPONENT(TableBody, "tbody", "c-table-body")
DEFINE_STYLED_HTML_COMPONENT(TableFooter, "tfoot", "c-table-footer")
DEFINE_STYLED_HTML_COMPONENT(TableRow, "tr", "c-table-row")
DEFINE_STYLED_HTML_COMPONENT(TableHead, "th", "c-table-head")
DEFINE_STYLED_HTML_COMPONENT(TableCell, "td", "c-table-cell")
DEFINE_STYLED_HTML_COMPONENT(TableCaption, "caption", "c-table-caption")
