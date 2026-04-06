#pragma once

#include <atomic>
#include <defines.hpp>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <vector>

#include "connection.hpp"
#include "router.hpp"

namespace http {

struct ServerCommand {
  std::string cmd;
  void parse();
};

struct TcpServer {
  TcpServer(char const *ip_address, i32 port, fp_connection *fn);
  ~TcpServer();

  void start_listen(bool async = false);
  void stop();

  void broadcast_message(std::vector<u8> const &message_bytes);

  fp_connection *m_connection_func;
  std::string m_ip_address;
  i32 m_port;
  i32 m_socket;
  struct sockaddr_in m_socket_address;
  std::vector<Connection *> m_active_connections;
  std::mutex m_connections_mutex;
  std::thread m_server_thread;
  std::atomic<bool> m_should_quit;
  bool m_running = false;
};

void http_client_thread(Connection *);
void ws_client_thread(Connection *);

struct HttpServer : TcpServer {
  HttpServer(char const *ip_address, i32 port)
      : TcpServer(ip_address, port, http_client_thread) {}

  Router &router() { return m_router; }
  Router m_router;
};

struct WsServer : TcpServer {
  WsServer(char const *ip_address, i32 port)
      : TcpServer(ip_address, port, ws_client_thread) {}

  Router &router() { return m_router; }
  Router m_router;
};

bool execute_server_command(http::TcpServer &serv,
                            http::ServerCommand const &cmd);

} // namespace http
