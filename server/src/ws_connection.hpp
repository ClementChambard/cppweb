#pragma once

#include <json/value.h>

void ws_open_connection(struct HttpServer *serv);
void ws_make_route(struct Router &r);
void ws_send_string(std::string const &v);
void ws_send_json(Json::Value v);
