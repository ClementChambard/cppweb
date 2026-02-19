#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::admin_sondages_rollers(http::Request r) {
  DECLARE_HTML(html, "root::layout") {
    CHILD("root::admin::sondages::rollers::page") {
      ITER_DB(s, id, RollersDb) {
        CHILD("components::rollers::admin_roller_card") {
          PARAM("id", id);
          PARAM("student", s->eleve);
          PARAM("size", s->size);
          PARAM_OPT("answered", s->has_answered);
          PARAM_OPT("has_roller", s->has_roller);
          PARAM_OPT("has_helmet", s->has_helmet);
          PARAM_OPT("has_protect", s->has_protect);
        }
      }
    }
  }
  return html;
}
