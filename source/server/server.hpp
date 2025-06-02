#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include <array>
#include <map>
#include <server/protocol.hpp>
#include <string>
#include <system/aliases.hpp>

/**
 * @brief Server configuration
 */
struct ServerConfig {
  u16 port = 8080;                    /** Port to listen on */
  u16 backlog = 10;                   /** Backlog for the listen socket */
  u32 max_message_size = 1024 * 1024; /** Maximum message size (1MB default) */
};

/**
 * @brief Server class
 */
class Server {
 public:
  /**
   * @brief Constructor
   * @param config Server configuration
   */
  Server(const ServerConfig& config = ServerConfig());

  /**
   * @brief Start the server
   * @note This function will block until the server is shutdown
   * @details Starts a loop that accepts client connections and handles requests
   *           until the server is shutdown. this server only handles one client
   *           at a time.
   */
  void start();

 private:
  /**
   * @brief Buffer type
   * @note This is a template that creates an array of characters with a
   *       specified size.
   */
  template <size_t N>
  using buffer = std::array<char, N>;

  /**
   * @brief Handle an error.
   * @throw std::runtime_error If an error occurs
   */
  void _handle_error();

  /**
   * @brief Read data from the client
   * @return True if the read was successful, false otherwise
   */
  bool _read();

  /**
   * @brief Parse the request
   * @param message The message to parse
   * @details This function will parse the request and set the request fields.
   */
  void _parse_request(const std::string& message);

  /**
   * @brief Parse the response
   * @return The response message
   * @details This function will parse the response and set the response fields.
   */
  std::string _parse_response();

  /**
   * @brief Handle the request
   * @details This function will handle the request and set the response fields.
   *          It will call the appropriate handler for the request.
   */
  void _handle_request();

  /**
   * @brief Handle the GET_ANSWERS request
   * @details This function will handle the GET_ANSWERS request. It will return
   *          all the answers in the AnswersManager.
   */
  void _handle_get_answers();

  /**
   * @brief Handle the SET_ANSWERS request
   * @details This function will handle the SET_ANSWERS request. It will set the
   *          answers in the AnswersManager (override).
   */
  void _handle_set_answers();

  /**
   * @brief Handle the REVIEW request
   * @details This function will handle the REVIEW request. It will send the
   *          exams to the workers for review.
   */
  void _handle_review();

  /**
   * @brief Handle the ECHO request
   * @details This function will handle the ECHO request. It will return the
   *          message received.
   */
  void _handle_echo();

  /**
   * @brief Handle the SHUTDOWN request
   * @details This function will handle the SHUTDOWN request. It will set the
   *          shutdown flag to true and send a shutdown signal to the workers.
   */
  void _handle_shutdown();

  /**
   * @brief Handle a bad request
   * @details This function will handle a bad request. It will set the response
   *          code to BAD_REQUEST and the response data to the error message.
   */
  void _handle_bad_request();

  i32 _client_socket_fd;       /** Client socket file descriptor */
  ServerConfig _config;        /** Server configuration */
  ScoreHiveRequest _request;   /** Request */
  ScoreHiveResponse _response; /** Response */
  i32 _mpi_size;               /** MPI size */
  bool _shutdown = false;      /** Shutdown flag */
};

#endif  // SERVER_HPP