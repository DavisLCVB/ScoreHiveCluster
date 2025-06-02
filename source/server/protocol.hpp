#pragma once
#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>
#include <system/aliases.hpp>

/**
 * @brief ScoreHive protocol commands
 * @details Each command represents a different action that can be 
 *          performed on the server.
 */
enum class ScoreHiveCommand : u8 {
  GET_ANSWERS = 0, /** Get answers from the server */
  SET_ANSWERS = 1, /** Set answers to the server */
  REVIEW = 2,      /** Review answers from the server */
  ECHO = 3,        /** Echo the data to the server */
  SHUTDOWN = 4     /** Shutdown the server */
};

enum class ScoreHiveResponseCode : u8 {
  OK = 0,    /** OK */
  ERROR = 1, /** Error */
};

static constexpr u8 MAX_COMMAND = 4; /** Maximum number of commands */

/**
 * @brief ScoreHive message. The message is used to communicate with the
 *        ScoreHive server.
 * @details The signatures of the commands are:
 *          - GET_ANSWERS: "SH 0$"
 *          - SET_ANSWERS: "SH 1 <length> <data>$"
 *          - REVIEW: "SH 2 <length> <data>$"
 *          - ECHO: "SH 3 <length> <data>$" 
 *          - SHUTDOWN: "SH 4$"
 */
struct ScoreHiveRequest {
  const char* magic = "SH"; /** Magic string of the message */
  ScoreHiveCommand command; /** Command to be performed */
  u32 length;               /** Length of the data */
  std::string data;         /** Incoming data */
};

/**
 * @brief ScoreHive response. The response is used to communicate with the
 *        ScoreHive server.
 */
struct ScoreHiveResponse {
  const char* magic = "SH";   /** Magic string of the message */
  ScoreHiveResponseCode code; /** Response code */
  u32 length;                 /** Length of the data */
  std::string data;           /** Outgoing data */
};

#endif  // PROTOCOL_HPP