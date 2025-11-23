#include "html/code_builder.hpp"
#include "routes.hpp"
#include <db.hpp>
#include <html/macros.hpp>

std::string page::root(http::Request) {
  DECLARE_HTML(html, "page::layout") {
    CHILD("page::root") {
      ITER_DB(s, id, SondagesDb) {
        CHILD("components::sondage_button") {
          PARAM("href", s->route.c_str());
          PARAM("text", s->button_text.c_str());
        }
      }
    }
  }
  return html;
}

// std::string functionally_equivalent_to() {
//   html::CodeBuilder layout("page::layout");
//   html::CodeBuilder page("page::root");
//   std::string page_children;
//   SondagesDb::lock();
//   for (auto &s : SondagesDb ::get().items) {
//     html::CodeBuilder button("components::sondage_button");
//     button.placeholder("href", s.route.c_str());
//     button.placeholder("text", s.button_text.c_str());
//     button.placeholder("children", "");
//     page_children += button.build();
//   }
//   SondagesDb ::unlock();
//   page.placeholder("children", page_children.c_str());
//   layout.placeholder("children", page.build().c_str());
//   return layout.build();
// }
