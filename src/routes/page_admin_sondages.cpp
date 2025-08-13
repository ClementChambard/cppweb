#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::admin_sondages(http::Request r) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::admin::sondages") {
      ITER_DB(s, id, SondagesDb) {
        CHILD("components::sondage_edit_card") {
          PARAM("id", id);
          PARAM("name", s->name);
          PARAM("button_text", s->button_text);
          PARAM("info", s->desc);
          PARAM_OPT("active", s->active);
        }
      }
    }
  }
  return html;
}
