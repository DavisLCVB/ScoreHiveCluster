#pragma once
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Environment class to manage environment variables
 */
class Environment {
 public:
  Environment() = delete;   // prevent instantiation
  ~Environment() = delete;  // prevent instantiation

  /**
   * @brief Get the value of an environment variable
   * @param key The key of the environment variable
   * @return std::optional<std::string> with the value of the environment
   *          variable if the environment variable is set, otherwise
   *          std::nullopt.
   * @details The environment variable is cached, so the first call to this
   *          function will be slower than the subsequent calls.
   */
  static std::optional<std::string> get(const std::string& key);

 private:
  /**
   * @brief The environment variables
   * @details The environment variables are cached here to avoid multiple calls
   *          to the getenv function.
   */
  static std::map<std::string, std::string> _env;
};

#endif  // ENVIRONMENT_HPP