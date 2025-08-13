#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::sondage_rdvs(http::Request r) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::sondage::layout") {
      PARAM("info", get_sondage_desc(r.endpoint.c_str()));
      ITER_DB(s, id, RdvsDb) {
        CHILD("components::rdvs::rdv_card") {
          auto h = std::to_string(s->heure) + 'h';
          if (h.size() == 1)
            h = '0' + h;
          auto m = std::to_string(s->minute);
          if (m.size() == 1)
            m = '0' + m;
          h += m;

          PARAM("id", id);
          PARAM("time", h);
          PARAM("student", s->eleve);
          PARAM_OPT("done", !s->eleve.empty());
        }
      }
    }
  }
  return html;
}
