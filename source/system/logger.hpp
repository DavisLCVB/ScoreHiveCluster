#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/spdlog.h>
#include <iostream>
#include <system/environment.hpp>

/**
 * @brief Logger class
 * @details This class is used to initialize the logging system.
 */
class Logger {
 public:
  /**
   * @brief Initialize the logging system.
   * @details This function is used to initialize the logging system. It sets
   *          the pattern and the level of the logging. It also sets the flush
   *          time to 5 seconds.
   * @see Environment::get()
   */
  static auto config(i32 rank) -> void {
    try {
      const auto debug_mode = Environment::get("DEBUG");  // Debug mode
      spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
      if (debug_mode && debug_mode.value() == "1") {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug mode is enabled for rank {}", rank);
      } else {
        spdlog::set_level(spdlog::level::info);
      }
      spdlog::flush_every(std::chrono::seconds(5));
      spdlog::info("Logging system initialized for rank {}", rank);
    } catch (const std::exception& e) {
      std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
      std::cerr << "Disabling logging" << std::endl;
      spdlog::set_level(spdlog::level::off);
    }
  }
};
#endif  // LOGGER_HPP