#include "../db.hpp"
#include "routes.hpp"
#include <algorithm>

http::Response api::rollers_id(http::Request r) {
  // TODO: can't change eleve if not authentified ?
  r.body_as_params();
  i32 idx = r.int_param("id");
  RollersDb::lock();
  auto old_eleve = RollersDb::get().items[idx].eleve;
  RollersDb::get().items[idx].eleve =
      r.string_param("student", RollersDb::get().items[idx].eleve.c_str());
  RollersDb::get().items[idx].size =
      r.int_param("size", RollersDb::get().items[idx].size);
  RollersDb::get().items[idx].has_protect =
      r.int_param("has_protect", RollersDb::get().items[idx].has_protect);
  RollersDb::get().items[idx].has_helmet =
      r.int_param("has_helmet", RollersDb::get().items[idx].has_helmet);
  RollersDb::get().items[idx].has_roller =
      r.int_param("has_roller", RollersDb::get().items[idx].has_roller);
  RollersDb::get().items[idx].has_answered =
      r.int_param("has_answered", RollersDb::get().items[idx].has_answered);
  if (old_eleve != RollersDb::get().items[idx].eleve)
    std::sort(RollersDb::get().items.begin(), RollersDb::get().items.end(),
              [](auto a, auto b) -> bool { return a.eleve < b.eleve; });
  RollersDb::get().write();
  RollersDb::unlock();
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .emptybody()
      .close()
      .build();
}

http::Response api::rollers(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  // TODO: if someone access to an element while a deletion is occuring, this
  // may cause problems.
  //       solution: use unique identifier for each item ?
  r.body_as_params();
  auto action = r.string_param("action");
  if (action == "delete") {
    i32 id = r.int_param("id", -1);
    RollersDb::lock();
    if (id < 0 || u32(id) >= RollersDb::get().items.size()) {
      RollersDb::unlock();
      return http::Response::bad_request();
    }
    RollersDb::get().items.erase(RollersDb::get().items.begin() + id);
    RollersDb::get().write();
    RollersDb::unlock();
  } else if (action == "create") {
    Rollers rollers;
    rollers.eleve = r.string_param("student");
    rollers.size = r.int_param("size");
    rollers.has_protect = r.int_param("has_protect");
    rollers.has_helmet = r.int_param("has_helmet");
    rollers.has_roller = r.int_param("has_roller");
    rollers.has_answered = r.int_param("has_answered");
    RollersDb::lock();
    RollersDb::get().items.push_back(rollers);
    std::sort(RollersDb::get().items.begin(), RollersDb::get().items.end(),
              [](auto a, auto b) -> bool { return a.eleve < b.eleve; });
    RollersDb::get().write();
    RollersDb::unlock();
  } else {
    return http::Response::bad_request();
  }
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .emptybody()
      .close()
      .build();
}
