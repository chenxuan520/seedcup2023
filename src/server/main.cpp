#include "game/game.h"
#include "logger.h"
#include "net/api.h"
#include "net/server.h"
#include <config.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <string>

void signal_handler(int /*signal_num*/) {
  // 刷新日志缓冲区
  // TODO

  // 退出程序
  exit(0);
}

int main() {
  signal(SIGINT, signal_handler);

  Config &config = Config::get_instance();

  // load configurations
  std::string logger_name = "seednet";
  auto host = config.get<std::string>("host");
  auto port = config.get<uint16_t>("port");
  std::string log_dir = "seedcup";
  std::vector<spdlog::sink_ptr> sinks;

  auto data = config.get_json();
  if (data.contains("black_list") && data["black_list"].is_array()) {
    for (const auto &item : data["black_list"]) {
      if (item.is_string()) {
        API::kBlackList.insert(item);
      }
    }
  }

  if (config.get<bool>("log_print_stdout")) {
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
  }
  sinks.push_back(
      std::make_shared<spdlog::sinks::daily_file_sink_st>(log_dir, 23, 59));

  // setup logger
  auto logger = setup_logger(logger_name, sinks);

  // set logger level
  spdlog::set_level(spdlog::level::level_enum::debug);
  logger->debug("setup logger success");

  auto epoll_server = std::make_shared<EpollTcpServer>(host, port, logger);
  if (!epoll_server) {
    logger->error("error occured when start epoll server");
    logger->flush();
    exit(EXIT_FAILURE);
  }

  auto recv_callback = API::handle_request;
  auto handle_timer_callback = API::handle_timer_intr;
  auto game_reset_callback = API::game_reset;
  epoll_server->register_on_recv_callback(recv_callback);
  epoll_server->register_on_handle_timer_callback(handle_timer_callback);
  epoll_server->register_on_game_reset_callback(game_reset_callback);

  if (!epoll_server->init()) {
    logger->error("epoll server cannot be init");
  }

  Game &game = Game::GetInstance();
  if (game.Init() != RC::SUCCESS) {
    logger->error("game core cannot be init");
  }

  // block here
  epoll_server->epoll_loop();

  return 0;
}
