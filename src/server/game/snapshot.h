#pragma once

#include "area.h"
#include "json.hpp"
#include <cstdio>
#include <fmt/format.h>
#include <memory.h>
#include <regex>
#include <string>
#include <utility>
#include <vector>

enum SnapshotEventType {
  SNAPSHOT_PLAYER_APPEAR,
  SNAPSHOT_PLAYER_DISAPPREAR,
  SNAPSHOT_BOMB_APPEAR,
  SNAPSHOT_BOMB_DISAPPREAR,
  SNAPSHOT_POTION_APPEAR,
  SNAPSHOT_POTION_DISAPPREAR,
  SNAPSHOT_BLOCK_APPEAR,
  SNAPSHOT_BLOCK_DISAPPREAR,
  SNAPSHOT_FLUSH,
};

const std::string kFormatString = "<{} {} [{} {}]>";
const std::regex pattern("<([^>]*)>");

class Snapshot {
public:
  enum StatusMachineStatus {
    STATUS_MACHINE_STATUS_UNINIT,
    STATUS_MACHINE_STATUS_NORMAL,
    STATUS_MACHINE_STATUS_END,
    STATUS_MACHINE_STATUS_ERROR,
  };
  Snapshot() = default;

  int Init(const std::string &log_str, std::string &err_msg) {
    std::sregex_iterator iter(log_str.begin(), log_str.end(), pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
      std::smatch match = *iter;
      std::string extracted = match.str(1);
      if (extracted.find("actionType") != std::string::npos) {
        continue;
      }
      log_arr_.push_back(extracted);
    }

    if (log_arr_.empty()) {
      err_msg = "log_arr_ is empty";
      return -1;
    }

    try {
      nlohmann::json json;
      json = nlohmann::json::parse(log_arr_[0]);
      std::vector<std::string> map_str = json["map"];
      if (map_str.empty()) {
        err_msg = "map_str is empty";
        return -1;
      }
      state_machine_.resize(map_str.size());
      for (int i = 0; i < map_str.size(); i++) {
        for (int j = 0; j < map_str[i].size(); j++) {
          std::shared_ptr<Area> area = std::make_shared<Area>(Pos{i, j});
          switch (map_str[i][j] - '0') {
          case MAP_OBJECT_BLOCK:
            area->set_block_id(1);
            break;
          case MAP_OBJECT_PLAYER:
            area->players().insert(1);
            break;
          case MAP_OBJECT_EMPTY:
            break;
          case MAP_OBJECT_UNKNOWN:
            break;
          default:
            err_msg = "map_str is invalid";
            return -1;
          }
          state_machine_[i].push_back(area);
        }
      }
    } catch (std::exception e) {
      err_msg = e.what();
      return -1;
    }
    err_msg = "";
    status_machine_status_ = STATUS_MACHINE_STATUS_NORMAL;
    now_step_++;
    return 0;
  }

  inline const std::vector<std::vector<std::shared_ptr<Area>>> &
  GetNowSnapshot() const {
    return state_machine_;
  }

  inline int GetNowRound() const { return now_round_; }
  inline StatusMachineStatus GetStatusMachineStatus() const {
    return status_machine_status_;
  }

  int NextRound(std::string &err_msg) {
    if (status_machine_status_ != STATUS_MACHINE_STATUS_NORMAL) {
      err_msg = "status_machine_status_ is not normal";
      return -1;
    }
    if (now_step_ >= log_arr_.size()) {
      err_msg = "now_round_ is out of range";
      return -1;
    }
    auto old_round = now_round_;
    while (now_round_ == old_round && now_step_ < log_arr_.size() &&
           status_machine_status_ == STATUS_MACHINE_STATUS_NORMAL) {
      auto rc = NextStep(err_msg);
      if (rc != 0) {
        return rc;
      }
    }
    return 0;
  }

  int NextStep(std::string &err_msg) {
    if (status_machine_status_ != STATUS_MACHINE_STATUS_NORMAL) {
      err_msg = "status_machine_status_ is not normal";
      return -1;
    }
    if (now_step_ >= log_arr_.size()) {
      err_msg = "now_round_ is out of range";
      return -1;
    }
    bool is_round = false;
    auto rc = FlushStatusMachine(log_arr_[now_step_], false, is_round, err_msg);
    if (rc != 0) {
      status_machine_status_ = STATUS_MACHINE_STATUS_ERROR;
      return rc;
    }
    if (is_round) {
      now_round_++;
    }
    now_step_++;
    if (now_step_ >= log_arr_.size()) {
      status_machine_status_ = STATUS_MACHINE_STATUS_END;
    }
    return 0;
  }

  int LastStep(std::string &err_msg) {
    if (now_step_ - 1 < 0) {
      err_msg = "now_step_ is out of range";
      return -1;
    }
    auto now_oper = log_arr_[now_step_ - 1];
    bool is_round = false;
    auto rc = FlushStatusMachine(now_oper, true, is_round, err_msg);
    if (rc != 0) {
      status_machine_status_ = STATUS_MACHINE_STATUS_ERROR;
      return rc;
    }
    if (is_round) {
      now_round_--;
    }
    now_step_--;
    return 0;
  }

  int LastRound(std::string &err_msg) {
    if (status_machine_status_ != STATUS_MACHINE_STATUS_NORMAL) {
      err_msg = "status_machine_status_ is not normal";
      return -1;
    }
    if (now_step_ >= log_arr_.size()) {
      err_msg = "now_round_ is out of range";
      return -1;
    }
    auto old_round = now_round_;
    if (now_round_ > 0 && IsRound(log_arr_[now_step_ - 1], err_msg)) {
      old_round--;
      now_round_--;
      now_step_--;
    }
    while (now_round_ == old_round && now_step_ > 0 &&
           status_machine_status_ == STATUS_MACHINE_STATUS_NORMAL) {
      auto rc = LastStep(err_msg);
      if (rc != 0) {
        return rc;
      }
    }
    return 0;
  }

public:
  static void GetFromatStringWithMap(
      const std::vector<std::vector<std::shared_ptr<Area>>> &map,
      std::string &result) {
    std::vector<std::string> arr_str(map.size(), "");
    for (int i = 0; i < map.size(); i++) {
      for (int j = 0; j < map[i].size(); j++) {
        if (map[i][j]->block_id() != -1) {
          arr_str[i] += std::to_string(MAP_OBJECT_BLOCK);
        } else if (map[i][j]->players().size() != 0) {
          arr_str[i] += std::to_string(MAP_OBJECT_PLAYER);
        } else if (map[i][j]->bomb_id() == -1 &&
                   map[i][j]->potion_type() == NO_POTION) {
          arr_str[i] += std::to_string(MAP_OBJECT_EMPTY);
        } else {
          arr_str[i] += std::to_string(MAP_OBJECT_UNKNOWN);
        }
      }
    }
    nlohmann::json json;
    json["map"] = arr_str;
    result = "<" + json.dump() + ">";
    return;
  }
  static void GetFormatString(SnapshotEventType event_type, int affect_id,
                              const std::pair<int, int> &pos,
                              std::string &result) {
    result = fmt::format(kFormatString, event_type, affect_id, pos.first,
                         pos.second);
  }

private:
  enum MapObjectType {
    MAP_OBJECT_EMPTY = 0,
    MAP_OBJECT_PLAYER = 1,
    MAP_OBJECT_BLOCK = 2,
    MAP_OBJECT_UNKNOWN = 3,
  };

private:
  std::vector<std::vector<std::shared_ptr<Area>>> state_machine_;
  StatusMachineStatus status_machine_status_ = STATUS_MACHINE_STATUS_UNINIT;
  std::vector<std::string> log_arr_;
  std::string log_text_;
  int now_step_ = 0;
  int now_round_ = 0;

private:
  bool IsRound(const std::string &oper, std::string &err_msg) {
    int oper_code = 0, oper_id = 0;
    std::pair<int, int> oper_pos = {0, 0};
    if (sscanf(oper.c_str(), "%d %d [%d %d]", &oper_code, &oper_id,
               &oper_pos.first, &oper_pos.second) != 4) {
      err_msg = "oper is invalid " + oper;
      return -1;
    }
    if (oper_pos.first < 0 || oper_pos.second < 0 ||
        oper_pos.first >= state_machine_.size() ||
        oper_pos.second >= state_machine_[oper_pos.first].size()) {
      err_msg = "oper_pos is invalid " + oper;
      return -1;
    }

    auto oper_type = static_cast<SnapshotEventType>(oper_code);
    return oper_type == SNAPSHOT_FLUSH;
  }

  int FlushStatusMachine(const std::string &oper, const bool is_reback,
                         bool &is_round, std::string &err_msg) {
    int oper_code = 0, oper_id = 0;
    std::pair<int, int> oper_pos = {0, 0};
    if (sscanf(oper.c_str(), "%d %d [%d %d]", &oper_code, &oper_id,
               &oper_pos.first, &oper_pos.second) != 4) {
      err_msg = "oper is invalid " + oper;
      return -1;
    }
    if (oper_pos.first < 0 || oper_pos.second < 0 ||
        oper_pos.first >= state_machine_.size() ||
        oper_pos.second >= state_machine_[oper_pos.first].size()) {
      err_msg = "oper_pos is invalid " + oper;
      return -1;
    }

    auto oper_type = static_cast<SnapshotEventType>(oper_code);
    if (is_reback) {
      switch (oper_type) {
      case SNAPSHOT_FLUSH:
        is_round = true;
        break;
      case SNAPSHOT_PLAYER_DISAPPREAR:
        oper_type = SNAPSHOT_PLAYER_APPEAR;
        is_round = false;
        break;
      case SNAPSHOT_BLOCK_DISAPPREAR:
        oper_type = SNAPSHOT_BLOCK_APPEAR;
        is_round = false;
        break;
      case SNAPSHOT_BOMB_DISAPPREAR:
        oper_type = SNAPSHOT_BOMB_APPEAR;
        is_round = false;
        break;
      case SNAPSHOT_POTION_DISAPPREAR:
        oper_type = SNAPSHOT_POTION_APPEAR;
        is_round = false;
        break;
      case SNAPSHOT_PLAYER_APPEAR:
        oper_type = SNAPSHOT_PLAYER_DISAPPREAR;
        is_round = false;
        break;
      case SNAPSHOT_BLOCK_APPEAR:
        oper_type = SNAPSHOT_BLOCK_DISAPPREAR;
        is_round = false;
        break;
      case SNAPSHOT_BOMB_APPEAR:
        oper_type = SNAPSHOT_BOMB_DISAPPREAR;
        is_round = false;
        break;
      case SNAPSHOT_POTION_APPEAR:
        oper_type = SNAPSHOT_POTION_DISAPPREAR;
        is_round = false;
        break;
      }
    }

    switch (oper_type) {
    case SNAPSHOT_FLUSH:
      is_round = true;
      break;
    case SNAPSHOT_PLAYER_DISAPPREAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->players().clear();
      is_round = false;
      break;
    case SNAPSHOT_BLOCK_DISAPPREAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_block_id(-1);
      is_round = false;
      break;
    case SNAPSHOT_BOMB_DISAPPREAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_bomb_id(-1);
      is_round = false;
      break;
    case SNAPSHOT_POTION_DISAPPREAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_potion_type(
          NO_POTION);
      is_round = false;
      break;
    case SNAPSHOT_PLAYER_APPEAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->players().insert(
          MAP_OBJECT_PLAYER);
      is_round = false;
      break;
    case SNAPSHOT_BOMB_APPEAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_bomb_id(
          oper_id);
      is_round = false;
      break;
    case SNAPSHOT_POTION_APPEAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_potion_type(
          static_cast<PotionType>(oper_id));
      is_round = false;
      break;
    case SNAPSHOT_BLOCK_APPEAR:
      this->state_machine_[oper_pos.first][oper_pos.second]->set_block_id(
          oper_id);
      is_round = false;
      break;
    }
    return 0;
  }
};
