#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::admin_sondages_piscine(http::Request r) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::admin::sondages::piscine") {
      ITER_DB(s, id, PiscineDb) {
        CHILD("components::piscine::admin_piscine_card") {
          PARAM("id", id);
          PARAM("parent", s->parent);
          PARAM("date", s->date);
          PARAM("date_formatted", format_date(s->date));
          PARAM_OPT("answered", !s->parent.empty());
        }
      }
    }
  }
  return html;
}
