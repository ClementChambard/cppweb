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

  SondagesDb sondages_db("private/sondages.db");
  RdvsDb rdvs_db("private/rdvs.db");
  PiscineDb piscine_db("private/piscine.db");
  RollersDb rollers_db("private/rollers.db");

  sys::info("====== DB OK ======");

  i32 port = std::stoi(sys::get_env_var("PORT", "8080"));
  http::TcpServer server("0.0.0.0", port);

  sys::info("====== DONE ======");

  server.router()
      // ======== PAGES ========
      .register_page("/")
      .register_page("/sondage/rdvs")
      .register_page("/sondage/piscine")
      .register_page("/sondage/rollers")
      .register_page_with_auth("/admin/sondages", is_authentified)
      .register_page_with_auth("/admin/sondages/rdvs", is_authentified)
      .register_page_with_auth("/admin/sondages/rollers", is_authentified)
      .register_page_with_auth("/admin/sondages/piscine", is_authentified)
      // ======== API ========
      // auth
      .post("/api/check_admin", api::check_admin)
      // sondages
      .get("/api/sondages", api::sondages::get)
      .put("/api/sondages/<id>", api::sondages::put)
      // piscine
      .get("/api/piscine", api::piscine::get)
      .post("/api/piscine", api::piscine::post)
      .put("/api/piscine/<id>", api::piscine::put)
      .del("/api/piscine/<id>", api::piscine::del)
      // rollers
      .get("/api/rollers", api::rollers::get)
      .post("/api/rollers", api::rollers::post)
      .put("/api/rollers/<id>", api::rollers::put)
      .del("/api/rollers/<id>", api::rollers::del)
      // rdvs
      .get("/api/rdvs", api::rdvs::get)
      .post("/api/rdvs", api::rdvs::post)
      .put("/api/rdvs/<id>", api::rdvs::put)
      .del("/api/rdvs/<id>", api::rdvs::del);

  server.start_listen();

  return 0;
}
