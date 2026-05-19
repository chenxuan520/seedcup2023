#include "server.h"

#include "config.h"
#include "socket/address.hpp"
#include "utils/const.hpp"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <unistd.h>

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

bool EpollTcpServer::init() {
  cppnet::Address addr{ip_, port_};
  server_.set_addr(addr);
  server_.set_max_connect_queue(kMaxConnecNum);

  auto rc = server_.Init();
  if (rc != cppnet::kSuccess) {
    logger_->error("server init failed: {}", server_.err_msg());
    return false;
  }
  if (auto mux = server_.io_multiplexing()) {
    mux->set_max_event_num(kMaxEventNum);
    if (kEpollTimeout > 0) {
      mux->set_wait_timeout(kEpollTimeout);
    }
  }

  // Arm the per-round timer. The config values are in milliseconds.
  int init_sec = kTimerInitTime / MICROSECS;
  int init_nsec = (kTimerInitTime % MICROSECS) * 1'000'000;
  int interval_sec = kTimerIntervalTime / MICROSECS;
  int interval_nsec = (kTimerIntervalTime % MICROSECS) * 1'000'000;
  (void)init_sec;
  (void)init_nsec;
  // cppnet TimerSocket uses one interval for both initial fire and reload,
  // matching what the original code actually did (both it_value and
  // it_interval were set to the same pair). We follow the same behaviour
  // and arm with the round interval.
  rc = timer_.Init(interval_sec, interval_nsec);
  if (rc != cppnet::kSuccess) {
    logger_->error("timer init failed");
    return false;
  }
  timer_fd_ = timer_.fd();

  rc = server_.AddSoc(timer_);
  if (rc != cppnet::kSuccess) {
    logger_->error("attach timer to server failed: {}", server_.err_msg());
    return false;
  }

  server_.Register(std::bind(&EpollTcpServer::on_event, this,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3));

  logger_->info("server init success on {}:{}", ip_, port_);
  return true;
}

bool EpollTcpServer::stop() {
  server_.Stop();
  // Best-effort wake up so the event loop returns from its blocking wait.
  server_.WakeUp();
  if (timer_fd_ >= 0) {
    timer_.Close();
    timer_fd_ = -1;
  }
  unregister_on_recv_callback();
  unregister_on_handle_timer_callback();
  unregister_on_game_reset_callback();
  logger_->info("server stopped");
  return true;
}

int EpollTcpServer::send_data(int fd, std::string msg) {
  if (fd < 0) {
    return -1;
  }
  uint64_t length = msg.size();
  // Pack length prefix + body into a single buffer for an atomic write,
  // matching the wire format the clients expect.
  std::string buf;
  buf.reserve(sizeof(length) + msg.size());
  buf.append(reinterpret_cast<const char *>(&length), sizeof(length));
  buf.append(msg);

  cppnet::Socket peer(fd);
  auto rc = peer.Write(buf);
  if (rc < 0) {
    logger_->error("write error on fd {}: {}", fd, peer.err_msg());
    return -1;
  }
  logger_->info("fd {} write {} bytes data", fd, rc);
  return rc;
}

void EpollTcpServer::register_on_recv_callback(callback_recv_t callback) {
  if (callback == nullptr) {
    logger_->warn("recv callback is nullptr");
  }
  recv_callback_ = std::move(callback);
}

void EpollTcpServer::register_on_handle_timer_callback(
    callback_handle_timer_t callback) {
  if (callback == nullptr) {
    logger_->warn("handle_timer callback is nullptr");
  }
  handle_timer_callback_ = std::move(callback);
}

void EpollTcpServer::register_on_game_reset_callback(
    callback_game_reset_t callback) {
  if (callback == nullptr) {
    logger_->warn("game_reset callback is nullptr");
  }
  game_reset_callback_ = std::move(callback);
}

void EpollTcpServer::unregister_on_recv_callback() {
  recv_callback_ = nullptr;
}
void EpollTcpServer::unregister_on_handle_timer_callback() {
  handle_timer_callback_ = nullptr;
}
void EpollTcpServer::unregister_on_game_reset_callback() {
  game_reset_callback_ = nullptr;
}

void EpollTcpServer::epoll_loop() {
  auto rc = server_.EventLoop();
  if (rc != cppnet::kSuccess) {
    logger_->error("event loop exited with error: {}", server_.err_msg());
  }
}

void EpollTcpServer::on_event(cppnet::TcpServer::Event event,
                              cppnet::TcpServer &server,
                              cppnet::Socket soc) {
  // Timer events are surfaced through the same Read channel; identify them
  // by fd so we don't pay the cost of a dedicated dispatcher.
  if (event == cppnet::TcpServer::kEventRead && soc.fd() == timer_fd_) {
    // Drain the 8-byte tick counter to make the fd readable-clear under LT
    // and consistent with Linux timerfd semantics.
    uint64_t ticks = 0;
    timer_.Read(&ticks, sizeof(ticks));
    on_timer_tick();
    return;
  }

  switch (event) {
  case cppnet::TcpServer::kEventAccept:
    on_accept(soc);
    break;
  case cppnet::TcpServer::kEventRead:
    on_read(server, soc);
    break;
  case cppnet::TcpServer::kEventLeave:
    on_leave(soc);
    break;
  case cppnet::TcpServer::kEventError:
    logger_->error("error on fd {}: {}", soc.fd(), server.err_msg());
    on_leave(soc);
    break;
  }
}

void EpollTcpServer::on_accept(cppnet::Socket &soc) {
  cppnet::Address peer;
  if (soc.GetAddr(peer) == cppnet::kSuccess) {
    logger_->info("accepting connection {} fd={}", peer.ToString(), soc.fd());
  } else {
    logger_->info("accepting connection fd={}", soc.fd());
  }
}

void EpollTcpServer::on_read(cppnet::TcpServer &server,
                             cppnet::Socket &soc) {
  // Wire format: uint64_t length (host byte order, matching the original
  // implementation) followed by `length` bytes of payload.
  uint64_t length = 0;
  auto rc = soc.Read(&length, sizeof(length), /*complete=*/true);
  if (rc <= 0) {
    if (rc == 0) {
      logger_->info("peer fd {} closed before sending header", soc.fd());
    } else {
      logger_->error("read header error on fd {}: {}", soc.fd(),
                     soc.err_msg());
    }
    server.RemoveSoc(soc);
    soc.Close();
    fd2PlayerId_.erase(soc.fd());
    fd2msg_.erase(soc.fd());
    return;
  }
  if (length == 0 || length >= 65535) {
    logger_->error("body length {} out of range on fd {}", length, soc.fd());
    server.RemoveSoc(soc);
    soc.Close();
    fd2PlayerId_.erase(soc.fd());
    fd2msg_.erase(soc.fd());
    return;
  }

  std::string body;
  rc = soc.Read(body, static_cast<size_t>(length), /*complete=*/true);
  if (rc <= 0) {
    logger_->error("read body error on fd {} ({} bytes expected): {}",
                   soc.fd(), length, soc.err_msg());
    server.RemoveSoc(soc);
    soc.Close();
    fd2PlayerId_.erase(soc.fd());
    fd2msg_.erase(soc.fd());
    return;
  }
  logger_->info("fd {} recv {} bytes: {}", soc.fd(), length, body);

  if (!recv_callback_) {
    return;
  }
  json parsed;
  try {
    parsed = json::parse(body);
  } catch (const std::exception &e) {
    logger_->error("json parse error on fd {}: {}", soc.fd(), e.what());
    return;
  }

  auto it = fd2PlayerId_.find(soc.fd());
  if (it != fd2PlayerId_.end()) {
    recv_callback_(parsed, it->second);
  } else {
    int player_id = -1;
    recv_callback_(parsed, player_id);
    if (player_id != -1) {
      fd2PlayerId_.emplace(soc.fd(), player_id);
    } else {
      logger_->error("adding player failed for fd {}", soc.fd());
    }
  }
}

void EpollTcpServer::on_leave(cppnet::Socket &soc) {
  logger_->info("peer fd {} disconnected", soc.fd());
  fd2PlayerId_.erase(soc.fd());
  fd2msg_.erase(soc.fd());
}

void EpollTcpServer::on_timer_tick() {
  if (!handle_timer_callback_) {
    return;
  }
  int retval = handle_timer_callback_(fd2msg_, fd2PlayerId_);
  for (auto &[fd, msg] : fd2msg_) {
    send_data(fd, msg);
  }
  if (retval == 1) {
    reset();
  }
}

bool EpollTcpServer::reset() {
  logger_->info("server reset");

  // Close all client connections; the timer fd stays attached.
  for (auto &[fd, _] : fd2PlayerId_) {
    cppnet::Socket peer(fd);
    server_.RemoveSoc(peer);
    peer.Close();
  }
  fd2PlayerId_.clear();
  fd2msg_.clear();

  if (game_reset_callback_ && !game_reset_callback_()) {
    logger_->error("game reset callback failed");
    return false;
  }
  // Re-arm the round timer with the configured interval. fd does not change.
  int interval_sec = kTimerIntervalTime / MICROSECS;
  int interval_nsec = (kTimerIntervalTime % MICROSECS) * 1'000'000;
  if (timer_.Reset(interval_sec, interval_nsec) != cppnet::kSuccess) {
    logger_->error("timer reset failed");
    return false;
  }
  return true;
}
