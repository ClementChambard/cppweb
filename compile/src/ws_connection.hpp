#pragma once

#include <html/html.hpp>
#include <json/value.h>

void ws_add_script_to_document(html::Document &doc);
void ws_open_connection();
void ws_close_connection();
void ws_send_string(std::string const &v);
void ws_send_json(Json::Value v);
