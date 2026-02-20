#include "../db.hpp"
#include "routes.hpp"

namespace api::rdvs {

http::Response get(http::Request r) {
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, RdvsDb) {
    Json::Value o{Json::ValueType::objectValue};
    o["id"] = s->id;
    o["eleve"] = s->eleve;
    o["minute"] = s->minute;
    o["heure"] = s->heure;
    o["txt"] = s->txt;
    out.append(o);
  }
  return http::Response::Builder()
      .code(200)
      .json(out)
      .close()
      .build();
}

http::Response put(http::Request r) {
  bool auth = is_authentified(r);
  auto v = r.body_as_json();
  if (v == std::nullopt) return http::Response::bad_request();
  auto body = *v;
  if (!body.isObject()) return http::Response::bad_request();

  RdvsDb::lock();
  auto rdv = RdvsDb::get().get_id(r.int_param("id", -1));
  if (!rdv) {
    RdvsDb::unlock();
    return http::Response::not_found();
  }
  if (auth) {
    rdv->heure = body.get_int("heure", rdv->heure);
    rdv->minute = body.get_int("minute", rdv->minute);
    rdv->txt = body.get_string("txt", rdv->txt.c_str());
  }
  if (auth || rdv->eleve == "") {
    rdv->eleve = body.get_string("eleve", rdv->eleve.c_str());
  }
  RdvsDb::get().write();
  RdvsDb::unlock();
  return http::Response::ok();
}

http::Response post(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  Json::Value o{Json::ValueType::objectValue};
  auto v = r.body_as_json();
  if (v == std::nullopt) return http::Response::bad_request();
  auto body = *v;
  if (!body.isObject()) return http::Response::bad_request();

  RdvsDb::lock();
  auto &rdv = RdvsDb::get().add_new();
  o["id"] = rdv.id;
  o["heure"] = rdv.heure = body.get_int("heure");
  o["minute"] = rdv.minute = body.get_int("minute");
  o["eleve"] = rdv.eleve = body.get_string("eleve");
  o["txt"] = rdv.txt = body.get_string("txt");
  RdvsDb::get().write();
  RdvsDb::unlock();
  return http::Response::Builder()
    .code(201)
    .json(o)
    .header("Location", (std::string("/api/rdvs/") + o["id"].asString()).c_str())
    .close()
    .build();
}

http::Response del(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  RdvsDb::lock();
  bool ok = RdvsDb::get().del_id(r.int_param("id", -1));
  RdvsDb::get().write();
  RdvsDb::unlock();
  if (!ok) return http::Response::not_found();
  return http::Response::ok();
}

} // namespace api::rdvs
