#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::root(http::Request) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::root") {
      ITER_DB(s, id, SondagesDb) {
        CHILD("components::sondage_button") {
          PARAM("href", s->route);
          PARAM("text", s->button_text);
        }
      }
    }
  }
  return html;
}
