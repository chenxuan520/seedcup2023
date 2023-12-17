#pragma once
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

// static const std::string logger_name = config.get<std::string>("loggerName");

inline std::shared_ptr<spdlog::logger>
setup_logger(const std::string &logger_name,
             std::vector<spdlog::sink_ptr> sinks) {
  auto logger = spdlog::get(logger_name);
  if (logger == nullptr) {
    if (sinks.size() > 0) {
      logger = std::make_shared<spdlog::logger>(logger_name, std::begin(sinks),
                                                std::end(sinks));
      spdlog::register_logger(logger);
    } else {
      logger = spdlog::stdout_color_mt(logger_name);
    }
  }
  return logger;
}

inline void test_logger(const std::string &logger_name, std::string message) {
  auto logger = spdlog::get(logger_name);
  if (logger) {
    logger->debug("{}: {}", __FUNCTION__, message);
  }
}
