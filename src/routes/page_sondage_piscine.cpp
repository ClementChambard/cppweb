#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>
#include <sstream>

std::string format_date(std::string date) {
  // yyyy-mm-dd => dd/mm/yyyy
  if (date.size() != 10)
    return date;
  std::ostringstream oss;
  oss << date[8] << date[9] << '/' << date[5] << date[6] << '/'
      << date.substr(0, 4);
  return oss.str();
}

std::string page::sondage_piscine(http::Request r) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::sondage::layout") {
      PARAM("info", get_sondage_desc(r.endpoint.c_str()));
      ITER_DB(s, id, PiscineDb) {
        CHILD("components::piscine::piscine_card") {
          PARAM("id", id);
          PARAM("parent", s->parent);
          PARAM("date_formatted", format_date(s->date));
          PARAM_OPT("done", !s->parent.empty());
        }
      }
    }
  }
  return html;
}
