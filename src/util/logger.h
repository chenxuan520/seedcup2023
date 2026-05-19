#pragma once
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <vector>

// fmt v9+ removed implicit enum-to-underlying conversion. Provide a generic
// formatter so existing code using "{}" with enums keeps compiling without
// touching every call site.
template <typename E>
struct fmt::formatter<E, std::enable_if_t<std::is_enum_v<E>, char>>
    : fmt::formatter<std::underlying_type_t<E>> {
  template <typename FormatContext>
  auto format(E value, FormatContext &ctx) const {
    return fmt::formatter<std::underlying_type_t<E>>::format(
        static_cast<std::underlying_type_t<E>>(value), ctx);
  }
};

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
