#include "server.hpp"
#include <mpi.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <array>
#include <cstring>
#include <domain/answers.hpp>
#include <domain/coordinator.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::json;

Server::Server(const ServerConfig& config) : _config(config) {}

void Server::start() {
  spdlog::info("Starting server...");
  // AF_INET: IPv4 protocol
  // SOCK_STREAM: TCP protocol
  // 0: Default protocol
  i32 socket_fd =
      socket(AF_INET, SOCK_STREAM, 0);  // Create the listening socket
  if (socket_fd == -1) {
    _handle_error();
  }
  MPI_Comm_size(MPI_COMM_WORLD, &_mpi_size);
  sockaddr_in address = {
      .sin_family = AF_INET,            // IPv4
      .sin_port = htons(_config.port),  // Port 8080
      .sin_addr = {htonl(INADDR_ANY)},  // Any IP address
      .sin_zero = {0}                   // Pad to size of `struct sockaddr'
  };
  sockaddr* address_ptr = reinterpret_cast<sockaddr*>(&address);
  // Bind the socket to the address and port
  auto bind_result = bind(socket_fd, address_ptr, sizeof(address));
  if (bind_result == -1) {
    _handle_error();
  }
  // Listen for incoming connections
  auto listen_result = listen(socket_fd, _config.backlog);
  if (listen_result == -1) {
    _handle_error();
  }
  while (!_shutdown) {
    spdlog::info("Server waiting for client on port 8080");
    // Accept an incoming connection
    auto accept_result = accept(socket_fd, nullptr, nullptr);
    if (accept_result == -1) {
      _handle_error();
      continue;
    }
    // Set the client socket file descriptor
    _client_socket_fd = accept_result;
    auto read_result = _read();
    if (!read_result) {
      _parse_response();
      auto response = _parse_response();
      auto send_result =
          send(_client_socket_fd, response.c_str(), response.size(), 0);
      if (send_result == -1) {
        _handle_error();
      }
      close(_client_socket_fd);
      continue;
    }
    spdlog::debug("Request received from client");
    _handle_request();
    auto response = _parse_response();
    auto send_result =
        send(_client_socket_fd, response.c_str(), response.size(), 0);
    if (send_result == -1) {
      _handle_error();
      close(_client_socket_fd);
      continue;
    }
    spdlog::debug("Response sent to client");
    close(_client_socket_fd);  // Close the client socket
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Sleep for 100ms to avoid busy-waiting
  }
}

void Server::_handle_error() {
  auto error_string = std::string(strerror(errno));
  spdlog::error("Failed to create socket: {}", error_string);
  throw std::runtime_error(error_string);
}

bool Server::_read() {
  try {
    std::string message;
    buffer<1024> buffer;
    // Read the message until the delimiter is found ('$' defined in the protocol)
    while (message.find('$') == std::string::npos) {
      auto recv_result =
          recv(_client_socket_fd, buffer.data(), buffer.size(), 0);
      if (recv_result == -1) {
        _handle_error();
      }
      if (recv_result == 0) {
        spdlog::error("Connection closed by client");
        return false;
      }
      message.append(buffer.data(), recv_result);
      if (message.size() > _config.max_message_size) {
        spdlog::error("Message size exceeds the maximum allowed size");
        return false;
      }
    }
    _parse_request(message);
    return true;
  } catch (std::exception& e) {
    spdlog::error("Failed to read data: {}", e.what());
    _response.code = ScoreHiveResponseCode::ERROR;
    _response.length = strlen(e.what());
    _response.data = e.what();
    return false;
  }
}

void Server::_parse_request(const std::string& message) {
  std::istringstream iss(message);
  std::string token;
  if (!std::getline(iss, token, ' ') || token != "SH") {
    throw std::runtime_error("Invalid magic string");
  }
  if (!std::getline(iss, token, ' ')) {
    throw std::runtime_error("Missing command");
  }
  size_t pos = token.find('$');
  if (pos != std::string::npos) {
    token = token.substr(0, pos);
  }
  u8 command = static_cast<u8>(std::stoi(token));
  if (command > MAX_COMMAND) {
    throw std::runtime_error("Invalid command");
  }
  if (command == 0) {
    _request.command = ScoreHiveCommand::GET_ANSWERS;
    _request.length = 0;
    _request.data = "";
    return;
  }
  if (command == 4) {
    _request.command = ScoreHiveCommand::SHUTDOWN;
    _request.length = 0;
    _request.data = "";
    return;
  }
  if (!std::getline(iss, token, ' ')) {
    throw std::runtime_error("Missing length");
  }
  u32 length = static_cast<u32>(std::stoi(token));
  if (length > _config.max_message_size) {
    throw std::runtime_error("Length exceeds the maximum allowed size");
  }
  if (!std::getline(iss, token, '$')) {
    throw std::runtime_error("Missing delimiter");
  }
  if (token.size() != length) {
    throw std::runtime_error("Data length mismatch");
  }
  _request.command = static_cast<ScoreHiveCommand>(command);
  _request.length = length;
  _request.data = token;
}

void Server::_handle_request() {
  switch (_request.command) {
    case ScoreHiveCommand::GET_ANSWERS:
      _handle_get_answers();
      break;
    case ScoreHiveCommand::SET_ANSWERS:
      _handle_set_answers();
      break;
    case ScoreHiveCommand::REVIEW:
      _handle_review();
      break;
    case ScoreHiveCommand::ECHO:
      _handle_echo();
      break;
    case ScoreHiveCommand::SHUTDOWN:
      _handle_shutdown();
      break;
    default:
      _handle_bad_request();
      break;
  }
}

void Server::_handle_get_answers() {
  auto data = AnswersManager::instance().save_to_json();
  _response.code = ScoreHiveResponseCode::OK;
  _response.length = data.size();
  _response.data = data;
}

void Server::_handle_set_answers() {
  auto data = json::parse(_request.data);
  try {
    AnswersManager::instance().load_from_json(data);
  } catch (std::exception& e) {
    std::string message = "Set Answers Error: " + std::string(e.what());
    spdlog::error(message);
    _response.code = ScoreHiveResponseCode::ERROR;
    _response.length = message.size();
    _response.data = message;
    return;
  }
  std::string message = "Set Answers OK";
  _response.code = ScoreHiveResponseCode::OK;
  _response.length = message.size();
  _response.data = message;
}

void Server::_handle_review() {
  auto exams_json = json::parse(_request.data);
  try {
    auto& coordinator = MPICoordinator::instance();
    coordinator.send_to_workers(exams_json, _mpi_size);
    auto results = coordinator.receive_results_from_workers(_mpi_size);
    auto msg = results.dump();
    _response.code = ScoreHiveResponseCode::OK;
    _response.length = msg.size();
    _response.data = msg;
  } catch (std::exception& e) {
    std::string message = "Review Error: " + std::string(e.what());
    spdlog::error(message);
    _response.code = ScoreHiveResponseCode::ERROR;
    _response.length = message.size();
    _response.data = message;
  }
}

void Server::_handle_echo() {
  auto data = _request.data;
  data = "Echo " + data;
  _response.code = ScoreHiveResponseCode::OK;
  _response.length = data.size();
  _response.data = data;
}

void Server::_handle_shutdown() {
  _shutdown = true;
  std::string message = "Server received shutdown signal";
  _response.code = ScoreHiveResponseCode::OK;
  _response.length = message.size();
  _response.data = message;
  spdlog::info(message);
  auto& coordinator = MPICoordinator::instance();
  coordinator.send_shutdown_signal(_mpi_size);
}

void Server::_handle_bad_request() {
  _response.code = ScoreHiveResponseCode::ERROR;
  _response.length = 0;
  _response.data = "Bad Request";
}

std::string Server::_parse_response() {
  std::string response = "SH";
  response += " ";
  response += std::to_string(static_cast<u8>(_response.code));
  response += " ";
  response += std::to_string(_response.length);
  response += " ";
  response += _response.data;
  response += "$\r\n";
  return response;
}