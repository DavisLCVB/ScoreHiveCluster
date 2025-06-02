#include "environment.hpp"

std::map<std::string, std::string> Environment::_env;

std::optional<std::string> Environment::get(const std::string& key) {
  //cached
  if (_env.find(key) != _env.end()) {
    return _env[key];
  }
  //not cached
  const auto value = std::getenv(key.c_str());
  if (value == nullptr) {
    return std::nullopt;
  }
  _env[key] = value;
  return value;
}