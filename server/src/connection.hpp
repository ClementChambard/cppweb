#pragma once

#include <defines.hpp>
#include <http/ws_connection.hpp>
#include <netinet/in.h>
#include <thread>

struct HttpServer;

void http_client_thread(struct Connection *self);

struct Connection {
  i32 m_socket;
  sockaddr_in m_socket_address;
  std::thread m_thread;
  HttpServer *m_server;
  http::WSConnection *ws = nullptr;

  void start_thread() { m_thread = std::thread(http_client_thread, this); }
};
