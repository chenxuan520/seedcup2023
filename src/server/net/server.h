#pragma once

#include "config.h"
#include "json.hpp"
#include "logger.h"
#include "server/tcp_server.hpp"
#include "socket/socket.hpp"
#include "timer/timer.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

// Round / timer interval is configured in milliseconds in config.json; this
// constant is kept for backward compatibility with code that still references
// it (e.g. game logic) and is also used by server.cpp itself when converting
// to (sec, nsec).
const int MICROSECS = 1000;

using callback_recv_t = std::function<void(json j, int &id)>;
using callback_handle_timer_t = std::function<int(
    std::unordered_map<int, std::string> &,
    std::unordered_map<int, int> &)>;
using callback_game_reset_t = std::function<bool()>;

/**
 * @brief: Tcp server with io-mux + timer driven game loop.
 *
 * Previously hand-rolled on top of <sys/epoll.h> + <sys/timerfd.h> (Linux-only
 * and fragile). The implementation has been swapped for cppnet which provides
 * the same epoll behaviour on Linux and falls back to kqueue / GCD on macOS,
 * making the server cross-platform without touching the API.
 *
 * The class name and existing callback signatures are intentionally
 * preserved so callers (main.cpp, api.cpp) do not have to change.
 */
class EpollTcpServer {
public:
  EpollTcpServer() = default;
  /**
   * @param ip: ip addr
   * @param port: port
   * @param logger: spdlog logger
   */
  EpollTcpServer(std::string ip, uint16_t port,
                 std::shared_ptr<spdlog::logger> logger)
      : ip_(std::move(ip)), port_(port), logger_(std::move(logger)) {}

  EpollTcpServer(const EpollTcpServer &) = delete;
  EpollTcpServer &operator=(const EpollTcpServer &) = delete;
  EpollTcpServer(EpollTcpServer &&) = delete;
  EpollTcpServer &operator=(EpollTcpServer &&) = delete;
  ~EpollTcpServer() { stop(); }

  /**
   * @brief: Init server (bind/listen) and arm the per-round timer.
   */
  bool init();
  /**
   * @brief: Stop the event loop and close all resources.
   */
  bool stop();
  /**
   * @brief: Send length-prefixed (uint64_t little-endian + body) message
   * to peer.
   */
  int send_data(int fd, std::string msg);
  /**
   * @brief: Register callback function when recv peer data.
   */
  void register_on_recv_callback(callback_recv_t callback);
  /**
   * @brief: Register callback function when timer ticks.
   */
  void register_on_handle_timer_callback(callback_handle_timer_t callback);
  /**
   * @brief: Register callback function fired on game reset.
   */
  void register_on_game_reset_callback(callback_game_reset_t callback);

  void unregister_on_recv_callback();
  void unregister_on_handle_timer_callback();
  void unregister_on_game_reset_callback();

  /**
   * @brief: Blocking event loop. Returns when stop() is called or a fatal
   * error occurs.
   *
   * Name kept for backward compatibility; the underlying implementation is
   * no longer raw epoll, but the semantics are the same.
   */
  void epoll_loop();

protected:
  static const int kMaxConnecNum;
  static const int kMaxEventNum;
  static const int kEpollTimeout;
  static const int kTimerInitTime;
  static const int kTimerIntervalTime;

private:
  void on_event(cppnet::TcpServer::Event event, cppnet::TcpServer &server,
                cppnet::Socket soc);
  void on_accept(cppnet::Socket &soc);
  void on_read(cppnet::TcpServer &server, cppnet::Socket &soc);
  void on_leave(cppnet::Socket &soc);
  void on_timer_tick();
  bool reset();

  std::string ip_;
  uint16_t port_{0};
  std::shared_ptr<spdlog::logger> logger_{nullptr};

  cppnet::TcpServer server_;
  cppnet::TimerSocket timer_;
  int timer_fd_{-1}; // cached timer_.fd() for fast comparison in events

  callback_recv_t recv_callback_{nullptr};
  callback_handle_timer_t handle_timer_callback_{nullptr};
  callback_game_reset_t game_reset_callback_{nullptr};

  // fd -> player id mapping, populated lazily on first InitReq for a fd
  std::unordered_map<int, int> fd2PlayerId_;
  // fd -> outbound message accumulated during the round, drained on each
  // timer tick
  std::unordered_map<int, std::string> fd2msg_;
};

using EpollTcpServerPtr = std::shared_ptr<EpollTcpServer>;
