#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::sondage_rollers(http::Request r) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::sondage::layout") {
      PARAM("info", get_sondage_desc(r.endpoint.c_str()));
      ITER_DB(s, id, RollersDb) {
        CHILD("components::rollers::roller_card") {
          PARAM("id", id);
          PARAM("student", s->eleve);
          PARAM("size", s->size);
          PARAM_OPT("has_roller", s->has_roller);
          PARAM_OPT("has_helmet", s->has_helmet);
          PARAM_OPT("has_protect", s->has_protect);
          PARAM_OPT("done", s->has_answered);
        }
      }
    }
  }
  return html;
}
