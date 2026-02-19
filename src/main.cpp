#include "html/page.hpp"
#include <db.hpp>
#include <http/router.hpp>
#include <http/tcp_server.hpp>
#include <routes/routes.hpp>
#include <string>
#include <sys/env.hpp>
#include <sys/logger.hpp>

int main() {
  sys::logger::initialize();

  sys::info("====== SERVER INITIALIZING ... ======");

  html::Page roller_page("roller", page::sondage_rollers);
  html::Page admin_rdv_page("admin_rdv", page::admin_sondages_rdvs);
  html::Page admin_roller_page("admin_roller", page::admin_sondages_rollers);
  html::Page admin_piscine_page("admin_piscine", page::admin_sondages_piscine);

  SondagesDb sondages_db("private/sondages.db");
  RdvsDb rdvs_db("private/rdvs.db");
  PiscineDb piscine_db("private/piscine.db");
  RollersDb rollers_db("private/rollers.db");

  rdvs_db.add_listener(admin_rdv_page);

  piscine_db.add_listener(admin_piscine_page);

  rollers_db.add_listener(roller_page);
  rollers_db.add_listener(admin_roller_page);

  i32 port = std::stoi(sys::get_env_var("PORT", "8080"));
  http::TcpServer server("0.0.0.0", port);

  server.router()
      .page("/")
      .page("/admin/sondages", is_authentified)
      .page("/admin/sondages/rdvs", admin_rdv_page, is_authentified)
      .page("/admin/sondages/rollers", admin_roller_page, is_authentified)
      .page("/admin/sondages/piscine", admin_piscine_page, is_authentified)
      .page("/sondage/rdvs")
      .page("/sondage/piscine")
      .page("/sondage/rollers", roller_page)
      .post("/api/check_admin", api::check_admin)
      .get("/api/sondages", api::get_sondages)
      .get("/api/sondages/<id>", api::get_sondage)
      .put("/api/sondages/<id>", api::put_sondage)
      .get("/api/rdvs", api::get_rdvs)
      .post("/api/rdvs/<id>", api::rdvs_id)
      .put("/api/rdvs/<id>", api::rdvs_id)
      .post("/api/rdvs", api::rdvs)
      .post("/api/piscine/<id>", api::piscine_id)
      .put("/api/piscine/<id>", api::piscine_id)
      .post("/api/piscine", api::piscine)
      .get("/api/piscine", api::get_piscine)
      .post("/api/rollers/<id>", api::rollers_id)
      .post("/api/rollers", api::rollers);

  server.start_listen();

  return 0;
}
