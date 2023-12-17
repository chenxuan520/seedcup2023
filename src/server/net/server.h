#pragma once

#include "config.h"
#include "logger.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <thread>
#include <vector>

const int MICROSECS = 1000;

// const int GameMaxFrame = Config::get_instance().get<int>("GameMaxFrame"); //
// nyw commented this line

/**
 * @brief: Basic unit of send data.
 */
class Packet {
public:
  int fd{-1};
  std::string msg;

  Packet() : msg("") {}
  Packet(const std::string &msg) : msg(msg) {}
  Packet(int fd, const std::string &msg) : fd(fd), msg(msg) {}
}; // NOTE(nyw):这个类好像没用

using callback_recv_t = std::function<void(json j, int &id)>;
using callback_handle_timer_t =
    std ::function<int(std::map<int, std::string> &, std::map<int, int> &)>;
using callback_game_reset_t = std::function<bool()>;

/**
 * @brief: Tcp server with epoll io mux.
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
      : ip_(ip), port(port), logger_(logger) {}

  EpollTcpServer(const EpollTcpServer &other) = delete;
  EpollTcpServer &operator=(const EpollTcpServer &other) = delete;
  EpollTcpServer(EpollTcpServer &&other) = delete;
  EpollTcpServer &operator=(EpollTcpServer &&other) = delete;
  ~EpollTcpServer() { stop(); };

  /**
   * @brief: Init tcp server. Create epfd, bind serverfd, set serverfd non
   * block and register serverfd to epoll.
   */
  bool init();
  /**
   * @brief: Closes tcp server. Close epfd and serverfd.
   */
  bool stop();
  /**
   * @brief: Tcp server send msg to peer.
   */
  int send_data(int fd, std::string msg);
  /**
   * @brief: Register callback function when recv peer data.
   */
  void register_on_recv_callback(callback_recv_t callback);
  /**
   * @brief: Register callback function when timer ticks.
   */
  void register_on_handle_timer_callback(
      callback_handle_timer_t callback); // nyw add
  /**
   * @brief: Unregister callback function of recv.
   */
  void unregister_on_recv_callback();
  /**
   * @brief: Unregister callback function of handle_timer.
   */
  void unregister_on_handle_timer_callback(); // nyw add
  /**
   * @brief: Epoll event loop.
   */

  void register_on_game_reset_callback(callback_game_reset_t callback);
  void unregister_on_game_reset_callback();

  void epoll_loop();

protected:
  int create_epoll();
  int create_socket();
  int create_timer();
  int set_socket_nonblock(int fd);
  int listen(int fd);
  int update_epoll_events(int efd, int op, int fd, int events);

  void handle_accept();
  void handle_read(int fd);
  void handle_write(int fd); // TODO(nyw):这玩意没用就删掉吧
  bool reset();
  bool reset_timer(int timerfd);
  // NOTE(nyw):这里用作每回合刷新game的状态，应该在api.h中实现，使server和game解耦，这里简单采用回调函数代替
  // void handle_timer(int fd);

  static const int kMaxConnecNum;
  static const int kMaxEventNum;
  static const int kEpollTimeout;
  static const int kTimerInitTime;
  static const int kTimerIntervalTime;

private:
  // Dot seperated ip address. eg: 127.0.0.1
  std::string ip_;
  // port
  uint16_t port{0};
  // epoll file descriptor
  int epfd_{-1};
  // server file descriptor
  int listenfd_{-1};
  // time file descriptor
  int timerfd_{-1};
  // one loop per thread
  std::shared_ptr<std::thread> th_{nullptr};
  // epoll loop flag, if false then exit
  bool loop_flag_{true};
  // callback function when recv data
  callback_recv_t recv_callbak_{nullptr};
  // callback function when timer ticks
  callback_handle_timer_t handle_timer_callbak_{nullptr};
  // callback function for game reset
  callback_game_reset_t game_reset_callbak_{nullptr};
  // logger for logs
  std::shared_ptr<spdlog::logger> logger_{nullptr};
};

typedef std::shared_ptr<EpollTcpServer> EpollTcpServerPtr;
