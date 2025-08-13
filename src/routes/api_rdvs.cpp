#include "../db.hpp"
#include "routes.hpp"
#include <algorithm>

http::Response api::rdvs_id(http::Request r) {
  // TODO: can't change eleve if it's already set and not authentified ?
  // TODO: can't change time if not authentified ?
  r.body_as_params();
  i32 idx = r.int_param("id");
  RdvsDb::lock();
  auto old_time =
      RdvsDb::get().items[idx].heure * 60 + RdvsDb::get().items[idx].minute;
  RdvsDb::get().items[idx].heure =
      r.int_param("hour", RdvsDb::get().items[idx].heure);
  RdvsDb::get().items[idx].minute =
      r.int_param("minute", RdvsDb::get().items[idx].minute);
  RdvsDb::get().items[idx].eleve =
      r.string_param("student", RdvsDb::get().items[idx].eleve.c_str());
  if (old_time !=
      RdvsDb::get().items[idx].heure * 60 + RdvsDb::get().items[idx].minute)
    std::sort(RdvsDb::get().items.begin(), RdvsDb::get().items.end(),
              [](auto a, auto b) -> bool {
                return a.heure < b.heure ||
                       (a.heure == b.heure && a.minute < b.minute);
              });
  RdvsDb::get().write();
  RdvsDb::unlock();
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .close()
      .build();
}

http::Response api::rdvs(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  // TODO: if someone access to an element while a deletion is occuring, this
  // may cause problems.
  //       solution: use unique identifier for each rdv ?
  r.body_as_params();
  auto action = r.string_param("action");
  if (action == "delete") {
    i32 id = r.int_param("id", -1);
    RdvsDb::lock();
    if (id < 0 || u32(id) >= RdvsDb::get().items.size()) {
      RdvsDb::unlock();
      return http::Response::bad_request();
    }
    RdvsDb::get().items.erase(RdvsDb::get().items.begin() + id);
    RdvsDb::get().write();
    RdvsDb::unlock();
  } else if (action == "create") {
    Rdv rdv;
    rdv.heure = r.int_param("hour");
    rdv.minute = r.int_param("minute");
    rdv.eleve = r.string_param("student");
    RdvsDb::lock();
    RdvsDb::get().items.push_back(rdv);
    std::sort(RdvsDb::get().items.begin(), RdvsDb::get().items.end(),
              [](auto a, auto b) -> bool {
                return a.heure < b.heure ||
                       (a.heure == b.heure && a.minute < b.minute);
              });
    RdvsDb::get().write();
    RdvsDb::unlock();
  } else {
    return http::Response::bad_request();
  }
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .close()
      .build();
}
