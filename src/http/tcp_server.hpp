#ifndef IG_HTTP_TCP_SERVER_HPP
#define IG_HTTP_TCP_SERVER_HPP

#include "../defines.hpp"
#include "http/router.hpp"
#include <atomic>
#include <string>
#include <netinet/in.h>
#include <thread>
#include <http/connection.hpp>
#include <mutex>
#include <vector>

namespace http {

struct TcpServer {
  TcpServer(char const *ip_address, i32 port);
  ~TcpServer();

  void start_listen();

  Router &router() { return m_router; }

  std::string m_ip_address;
  i32 m_port;
  i32 m_socket;
  struct sockaddr_in m_socket_address;
  std::vector<Connection*> m_active_connections;
  std::mutex m_connections_mutex;
  std::thread m_server_thread;
  std::atomic<bool> m_should_quit;
  Router m_router;
};

} // namespace http

#endif // !IG_HTTP_TCP_SERVER_HPP
