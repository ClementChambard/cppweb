#include "http_server.hpp"
#include "connection.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/logger.hpp>
#include <sys/socket.h>
#include <unistd.h>

void ServerCommand::parse() {}

bool execute_server_command(ServerCommand const &cmd) {
  if (cmd.cmd == "/stop") {
    return true;
  }
  if (cmd.cmd == "/help") {
    sys::info("[/help]: Available commands:\n"
              " - /stop: stops the server\n"
              " - /help: show this message\n");
    return false;
  }
  return false;
}

HttpServer::HttpServer(char const *ip_address, i32 port)
    : m_ip_address(ip_address), m_port(port), m_socket(), m_socket_address(),
      m_should_quit(false) {
  m_socket_address.sin_family = AF_INET;
  m_socket_address.sin_port = htons(m_port);
  m_socket_address.sin_addr.s_addr = inet_addr(ip_address);

  m_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket < 0) {
    // TOOD: log
    sys::fatal_error("Cannot create socket");
  }

  if (bind(m_socket, (sockaddr *)&m_socket_address, sizeof(m_socket_address)) <
      0) {
    sys::fatal_error("Cannot connect socket to address %s:%d", ip_address,
                     port);
  }
}

HttpServer::~HttpServer() { stop(); }

void server_thread_run(HttpServer *self) {
  socklen_t client_addr_size = sizeof(sockaddr_in);

  Connection *client = new Connection;
  while (true) {
    client->m_server = self;
    client->m_socket =
        accept(self->m_socket, (sockaddr *)&client->m_socket_address,
               &client_addr_size);
    if (client->m_socket < 0) {
      if (self->m_should_quit)
        break;
      sys::error("Accept failed");
      continue;
    }
    self->m_connections_mutex.lock();
    self->m_active_connections.push_back(client);
    client->start_thread();
    self->m_connections_mutex.unlock();
    client = new Connection;
  }
  delete client;
}

void server_console_loop() {
  while (true) {
    ServerCommand cmd;
    std::getline(std::cin, cmd.cmd, '\n');
    cmd.parse();
    if (execute_server_command(cmd)) {
      break;
    }
  }
}

void HttpServer::start_listen(bool async) {
  m_running = true;
  if (listen(m_socket, 20) < 0) {
    sys::fatal_error("Socket listen failed");
  }

  sys::info("*** Listening on ADDRESS: %s PORT: %d ***", m_ip_address.c_str(),
            m_port);

  m_server_thread = std::thread(server_thread_run, this);

  if (!async) {
    server_console_loop();
  }
}

void HttpServer::stop() {
  if (!m_running)
    return;
  m_running = false;
  m_should_quit = true;
  sys::info("BEFORE SHUTDOWN");
  shutdown(m_socket, SHUT_RDWR); // this does not shutdown the socket ????
  sys::info("SHUTDOWN");
  close(m_socket);
  m_server_thread.join();
  m_connections_mutex.lock();
  for (auto const &client : m_active_connections) {
    delete client;
  }
  m_connections_mutex.unlock();
  sys::info("Server stopped");
}

void HttpServer::ws_broadcast_message(char const *origin,
                                      std::vector<u8> const &message_bytes) {
  for (auto c : m_active_connections) {
    if (c->ws && c->ws->origin_endpoint == origin)
      c->ws->send_message(message_bytes.data(), message_bytes.size());
  }
}
