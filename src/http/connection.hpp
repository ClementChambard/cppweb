#ifndef IG_HTTP_CONNECTION_HPP
#define IG_HTTP_CONNECTION_HPP

#include <defines.hpp>
#include <netinet/in.h>
#include <thread>

namespace http {

struct TcpServer;

struct Connection {
  i32 m_socket;
  sockaddr_in m_socket_address;
  std::thread m_thread;
  TcpServer *m_server;

  void start_thread();
};

} // namespace http

#endif // !IG_HTTP_CONNECTION_HPP
