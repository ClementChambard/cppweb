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

struct ServerCommand {
  std::string cmd;
  void parse();
};

struct HttpServer {
  HttpServer(char const *ip_address, i32 port);
  ~HttpServer();

  void start_listen(bool async = false);
  void stop();

  void ws_broadcast_message(char const *origin,
                            std::vector<u8> const &message_bytes);

  std::string m_ip_address;
  i32 m_port;
  i32 m_socket;
  struct sockaddr_in m_socket_address;
  std::vector<Connection *> m_active_connections;
  std::mutex m_connections_mutex;
  std::thread m_server_thread;
  std::atomic<bool> m_should_quit;
  bool m_running = false;

  Router &router() { return m_router; }
  Router m_router;
};

void http_client_thread(Connection *);

bool execute_server_command(ServerCommand const &cmd);
