#include "../db.hpp"
#include "http/response.hpp"
#include "routes.hpp"

namespace api::rollers {

http::Response get(http::Request r) {
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, RollersDb) {
    Json::Value o{Json::ValueType::objectValue};
    o["id"] = s->id;
    o["eleve"] = s->eleve;
    o["size"] = s->size;
    o["has_roller"] = s->has_roller;
    o["has_helmet"] = s->has_helmet;
    o["has_protect"] = s->has_protect;
    o["has_answered"] = s->has_answered;
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

  RollersDb::lock();
  auto roller = RollersDb::get().get_id(r.int_param("id", -1));
  if (!roller) {
    RollersDb::unlock();
    return http::Response::not_found();
  }
  if (auth) {
    roller->eleve = body.get_string("eleve", roller->eleve.c_str());
  }
  roller->size = body.get_int("size", roller->size);
  roller->has_protect = body.get_bool("has_protect", roller->has_protect);
  roller->has_helmet = body.get_bool("has_helmet", roller->has_helmet);
  roller->has_roller = body.get_bool("has_roller", roller->has_roller);
  roller->has_answered = body.get_bool("has_answered", roller->has_answered);
  RollersDb::get().write();
  RollersDb::unlock();
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

  RollersDb::lock();
  auto &roller = RollersDb::get().add_new();
  o["id"] = roller.id;
  o["eleve"] = roller.eleve = body.get_string("eleve");
  o["size"] = roller.size = body.get_int("size");
  o["has_protect"] = roller.has_protect = body.get_bool("has_protect");
  o["has_helmet"] = roller.has_helmet = body.get_bool("has_helmet");
  o["has_roller"] = roller.has_roller = body.get_bool("has_roller");
  o["has_answered"] = roller.has_answered = body.get_bool("has_answered");
  RollersDb::get().write();
  RollersDb::unlock();
  return http::Response::Builder()
    .code(201)
    .json(o)
    .header("Location", (std::string("/api/rollers/") + o["id"].asString()).c_str())
    .close()
    .build();
}

http::Response del(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  RollersDb::lock();
  bool ok = RollersDb::get().del_id(r.int_param("id", -1));
  RollersDb::get().write();
  RollersDb::unlock();
  if (!ok) return http::Response::not_found();
  return http::Response::ok();
}

} // namespace api::rollers
