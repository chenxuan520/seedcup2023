#include "server.h"
#include "config.h"
// #include "game/game.h"
// #include "game/player.h"
// #include "game/rc.h"
// #include "net/api.h"
#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <unordered_map>

static std::unordered_map<int, int> fd2PlayerId;
static std::unordered_map<int, std::string> fd2msg;

const int EpollTcpServer::kMaxConnecNum =
    Config::get_instance().get<int>("server_max_connection_num");
const int EpollTcpServer::kMaxEventNum =
    Config::get_instance().get<int>("epoll_max_events_num");
const int EpollTcpServer::kEpollTimeout =
    Config::get_instance().get<int>("epoll_timeout");
const int EpollTcpServer::kTimerInitTime =
    Config::get_instance().get<int>("timer_initial_value");
const int EpollTcpServer::kTimerIntervalTime =
    Config::get_instance().get<int>("round_interval_value");

int EpollTcpServer::create_socket() {
  int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    logger_->error("create socket {}:{} failed.", ip_, port);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_.c_str());

  // set reuse addr to avoid time wait delay
  int reuse_addr_on = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_on,
                 sizeof(reuse_addr_on)) < 0) {
    logger_->error("set SO_REUSEADDR error");
    ::close(listenfd);
    exit(-1);
  }

  if (::bind(listenfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
    logger_->error("bind socket {}:{} failed.", ip_, port);
    ::close(listenfd);
    exit(-1);
  }

  logger_->info("create and bind socket {}:{} success!", ip_, port);

  return listenfd;
}

int EpollTcpServer::create_timer() {
  int timerfd = timerfd_create(CLOCK_REALTIME, 0);
  if (timerfd <= 0) {
    logger_->info("timerfd create failed!");
    return -1;
  }

  reset_timer(timerfd);
  return timerfd;
}

bool EpollTcpServer::reset_timer(int timerfd) {
  struct itimerspec ts;
  ts.it_value.tv_sec = kTimerInitTime / MICROSECS;
  ts.it_value.tv_nsec = kTimerInitTime % MICROSECS * 1e6;
  ts.it_interval.tv_sec = kTimerIntervalTime / MICROSECS;
  ts.it_interval.tv_nsec = kTimerIntervalTime % MICROSECS * 1e6;

  timerfd_settime(timerfd, 0, &ts, NULL);
  return true;
}

int EpollTcpServer::set_socket_nonblock(int fd) {

  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    logger_->error("fcntl failed.");
    return -1;
  }

  int retval = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (retval < 0) {
    logger_->error("fcntl failed.");
    return -1;
  }

  return 0;
}

int EpollTcpServer::listen(int fd) {
  int retval = ::listen(fd, kMaxConnecNum);
  if (retval < 0) {
    logger_->error("listen failed.");
    return -1;
  }

  return 0;
}

int EpollTcpServer::create_epoll() {
  int epfd = epoll_create1(0);
  if (epfd < 0) {
    logger_->error("epoll create failed.");
    return -1;
  }

  return epfd;
}

int EpollTcpServer::delete_epoll_events(int efd, int fd) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(epoll_event));

  int retval = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
  if (retval < 0) {
    logger_->error("epoll ctl failed.");
    return -1;
  }

  return 0;
}

int EpollTcpServer::update_epoll_events(int efd, int op, int fd, int events) {

  struct epoll_event ev;
  memset(&ev, 0, sizeof(epoll_event));

  ev.events = events;
  // TODO(gpl): handler param
  ev.data.fd = fd;
  logger_->info("Epoll op: {}, Epoll events: {}.",
                op == EPOLL_CTL_ADD ? "add" : "mod",
                (events & EPOLLIN) > 0    ? "read"
                : (events & EPOLLOUT) > 0 ? "write"
                                          : "undentified");

  int retval = epoll_ctl(efd, op, fd, &ev);
  if (retval < 0) {
    logger_->error("epoll ctl failed.");
    return -1;
  }

  return 0;
}

bool EpollTcpServer::init() {
  epfd_ = create_epoll();
  if (epfd_ < 0) {
    return false;
  }

  timerfd_ = create_timer();
  if (timerfd_ < 0) {
    return false;
  }

  listenfd_ = create_socket();
  if (listenfd_ < 0) {
    return false;
  }

  int retval = set_socket_nonblock(listenfd_);
  if (retval < 0) {
    return false;
  }

  retval = listen(listenfd_);
  if (retval < 0) {
    return false;
  }

  logger_->info("EpollTcpServer Init Success.");

  retval =
      update_epoll_events(epfd_, EPOLL_CTL_ADD, listenfd_, EPOLLIN | EPOLLET);
  if (retval < 0) {
    ::close(epfd_);
    return false;
  }

  retval =
      update_epoll_events(epfd_, EPOLL_CTL_ADD, timerfd_, EPOLLET | EPOLLIN);
  if (retval < 0) {
    ::close(epfd_);
    return false;
  }

  return true;
}

bool EpollTcpServer::stop() {
  loop_flag_ = false;
  ::close(listenfd_);
  ::close(timerfd_);
  ::close(epfd_);
  logger_->info("epoll tcp server stoped.");
  unregister_on_recv_callback();
  unregister_on_handle_timer_callback();
  return true;
}

bool EpollTcpServer::reset() {
  logger_->info("epoll tcp server reset.");

  // clean old connect
  for (auto [fd, _] : fd2PlayerId) {
    close_fd(fd);
  }

  fd2PlayerId.clear();
  fd2msg.clear();

  // reset game
  if (!game_reset_callbak_()) {
    logger_->error("game reset error.");
    return false;
  }

  if (!reset_timer(timerfd_)) {
    logger_->error("epoll tcp server init error.");
    return false;
  }

  return true;
}

int EpollTcpServer::send_data(int fd, std::string msg) {
  if (fd < 0) {
    return -1;
  }

  uint64_t length = msg.size();
  int retval = ::write(fd, &length, sizeof(length));
  if (retval < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return -1;
    }
    logger_->error("write header error occured on fd {}.", fd);
    close_fd(fd);
  }

  retval = ::write(fd, msg.data(), msg.size());
  if (retval < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return -1;
    }
    logger_->error("write error occured on fd {}.", fd);
    close_fd(fd);
  }

  logger_->info("fd {} write {} bytes data.", fd, retval);

  return retval;
}

int EpollTcpServer::close_fd(int fd) {
  if (fd < 0) {
    return -1;
  }

  delete_epoll_events(epfd_, fd);
  ::close(fd);
  return 0;
}

void EpollTcpServer::register_on_recv_callback(callback_recv_t callback) {
  if (callback == nullptr) {
    logger_->warn("callback function of recv data is nullptr.");
  }
  recv_callbak_ = callback;
  return;
}

void EpollTcpServer::register_on_handle_timer_callback(
    callback_handle_timer_t callback) { // nyw add
  if (callback == nullptr) {
    logger_->warn("callback function of handle_timer is nullptr.");
  }
  handle_timer_callbak_ = callback;
  return;
}

void EpollTcpServer::register_on_game_reset_callback(
    callback_game_reset_t callback) {
  if (callback == nullptr) {
    logger_->warn("callback function of game_reset is nullptr.");
  }
  game_reset_callbak_ = callback;
  return;
}

void EpollTcpServer::unregister_on_recv_callback() {
  recv_callbak_ = nullptr;
  return;
}

void EpollTcpServer::unregister_on_game_reset_callback() {
  game_reset_callbak_ = nullptr;
  return;
}

void EpollTcpServer::unregister_on_handle_timer_callback() { // nyw add
  handle_timer_callbak_ = nullptr;
  return;
}

void EpollTcpServer::handle_accept() {

  while (true) {

    struct sockaddr_in in_addr;
    socklen_t in_len = sizeof(in_addr);

    int cli_fd = accept(listenfd_, (struct sockaddr *)&in_addr, &in_len);
    if (cli_fd < 0) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        logger_->info("accepting all connections.");
        break;
      } else {
        logger_->error("accept error.");
        continue;
      }
    }

    sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int retval = getpeername(cli_fd, (sockaddr *)&peer_addr, &peer_len);
    if (retval < 0) {
      logger_->error("getpeername error.");
      continue;
    }

    logger_->info("accepting connection from {}.",
                  inet_ntoa(peer_addr.sin_addr));

    retval = set_socket_nonblock(cli_fd);
    if (retval < 0) {
      logger_->error("can not set socket {} to non block.", cli_fd);
      ::close(cli_fd);
      continue;
    }

    retval = update_epoll_events(epfd_, EPOLL_CTL_ADD, cli_fd,
                                 EPOLLIN | EPOLLET | EPOLLRDHUP);
    if (retval < 0) {
      logger_->error("can not add socket {} to epoll.", cli_fd);
      ::close(cli_fd);
      continue;
    }
  }
  return;
}

void EpollTcpServer::handle_read(int fd) {

  if (fd < 0) {
    logger_->error("fd must greater than 0");
    return;
  }

  uint64_t length = 0;
  if (::read(fd, &length, sizeof(length)) < sizeof(length)) {
    logger_->error("read header error on fd {}", fd);
    return;
  }

  if (length >= 65535) {
    logger_->error("body length {} too large on fd {}", length, fd);
    return;
  }

  char buffer[length];

  bzero(buffer, sizeof(buffer));

  int offset = 0;
  int size = -1;

  while (offset < length) {
    while ((size = ::read(fd, buffer + offset, length - offset)) > 0) {
      offset += size;
    }
  }

  if (size == -1) {
    // finished reading bytes
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    logger_->error("something went wrong for fd {}.", fd);
    close_fd(fd);
    return;
  }

  if (size == 0 && offset != length) {
    logger_->info("client fd: {} close socket.", fd);
    close_fd(fd);
    return;
  }

  std::string msg(buffer, length);
  logger_->info("fd {} recv {} bytes, content: {}", fd, length, msg);

  if (recv_callbak_) {
    if (fd2PlayerId.count(fd)) {
      recv_callbak_(json::parse(msg), fd2PlayerId[fd]);
    } else {
      int player_id = -1; // nyw add to specify new InitReq
      recv_callbak_(json::parse(msg), player_id);
      if (player_id != -1) // adding player succeed
      {
        fd2PlayerId.insert({fd, player_id});
      } else {
        logger_->error("adding player fail");
      }
    }
  }

  return;
}

void EpollTcpServer::handle_write(int fd) {
  // TODO(gpl): handle write
  return;
}

void EpollTcpServer::epoll_loop() {
  struct epoll_event ready_events[kMaxEventNum];
  bzero(ready_events, sizeof(ready_events));

  while (loop_flag_) {
    int num = epoll_wait(epfd_, (epoll_event *)&ready_events, kMaxEventNum,
                         kEpollTimeout);

    for (int i = 0; i < num; ++i) {
      int fd = ready_events[i].data.fd;
      int events = ready_events[i].events;

      if ((events & EPOLLERR) || (events & EPOLLHUP)) {
        logger_->info("error occured to fd {}.", fd);
        close_fd(fd);
      } else if (events & EPOLLRDHUP) {
        // Stream socket peer closed connection, or shut down writing half of
        // connection.
        logger_->info("Stream socket peer {} closed connections.", fd);
        close_fd(fd);
      } else if (events & EPOLLIN) {
        if (fd == listenfd_) {
          handle_accept();
        } else if (fd == timerfd_) {
          // logger_->info("timer {} ticks", timerfd_);
          int64_t data;
          int retval = ::read(timerfd_, &data, sizeof(data));
          // MODIFIED(nyw): use handle timer call back function
          retval = handle_timer_callbak_(fd2msg, fd2PlayerId);
          for (auto &[fd, msg] : fd2msg) {
            send_data(fd, msg);
          }
          if (retval == 1) // game over
          {
            reset();
          }
        } else {
          handle_read(fd);
        }
      } else if (events & EPOLLOUT) {
        handle_write(fd);
      } else {
        logger_->error("unknown epoll events.");
      }
    }
  }

  return;
}
