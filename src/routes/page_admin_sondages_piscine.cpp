#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string format_date(std::string date) {
  // yyyy-mm-dd => dd/mm/yyyy
  if (date.size() != 10)
    return date;
  std::ostringstream oss;
  oss << date[8] << date[9] << '/' << date[5] << date[6] << '/'
      << date.substr(0, 4);
  return oss.str();
}

std::string page::admin_sondages_piscine(http::Request r) {
  DECLARE_HTML(html, "root::layout") {
    CHILD("root::admin::sondages::piscine::page") {
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
