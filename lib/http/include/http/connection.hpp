#pragma once

#include <defines.hpp>
#include <netinet/in.h>
#include <thread>

namespace http {

struct TcpServer;

using fp_connection = void(struct Connection*);

struct Connection {
  i32 m_socket;
  sockaddr_in m_socket_address;
  std::thread m_thread;
  TcpServer *m_server;

  void start_thread(fp_connection *fn) { m_thread = std::thread(fn, this); }
};

} // namespace http
