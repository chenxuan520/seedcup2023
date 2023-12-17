#include "api.h"
#include "../game/const.h"
#include "../game/game.h"
#include "../game/player.h"
#include "json.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <vector>

namespace API {

std::string kResultPath = "";

std::unordered_set<std::string> kBlackList;

void to_json(json &j, const InitReq &req) {
  j = json{}; // NOTE(nyw):InitReq暂时不需要参数
}

void to_json(json &j, const ActionReq &req) {
  j = json{{"playerID", req.playerID}, {"actionType", req.action_type}};
}

void from_json(const json &j, InitReq &req) {
  // NOTE(nyw):there is nothing todo
}

void from_json(const json &j, ActionReq &req) {
  req.playerID = j["playerID"];
  req.action_type = j["actionType"].get<ActionType>();
}

/*
 * brief: 收到client的InitReq时调用,作用是添加一名玩家
 * return: new player id
 */
void handle_init(int &player_id, const std::string &player_name) {
  // 判断是否是黑名单成员,直接拒绝
  if (kBlackList.find(player_name) != kBlackList.end()) {
    // TODO:这里应该的日志部分有待商榷,这样弄不是很好
    spdlog::default_logger()->error("{} player is in blackList", player_name);
    return;
  }
  // NOTE(nyw):由于本次游戏一个玩家只能有一名角色,这部分实现会比较简单
  Game &game = Game::GetInstance();
  int retval = game.AddPlayer(player_id, player_name);
  if (retval != RC::SUCCESS) {
    player_id = -1;
  }
}

void handle_action(const json &data, const int &player_id) {
  Game &game = Game::GetInstance();

  if (game.GetPlayerByID(player_id) == nullptr) {
    return;
  }

  ::Action action;
  action.player_id_ = player_id;
  action.action_type_ = data.get<ActionReq>().action_type;

  int retval = game.AddAction(action);
}

void handle_request(json req, int &player_id) {
  if (req.empty()) {
    return;
  }

  Game &game = Game::GetInstance();

  try {
    if (req.contains("type") && req.contains("data")) {
      if (req["type"].get<PacketType>() == PacketType::Init &&
          player_id == -1) {
        handle_init(player_id, req["data"]["player_name"].get<std::string>());
      } else if (req["type"].get<PacketType>() == PacketType::Action &&
                 game.game_status() == GameStatus::WAIT_ACTION) {
        if (!req["data"].is_array()) {
          handle_action(req["data"], player_id);
        } else {
          for (int i = 0; i < req["data"].size(); i++) {
            handle_action(req["data"][i], player_id);
          }
        }
      }
    }
  } catch (std::exception e) {
    return;
  }
  return;
}

json customed_to_json(::Player &player) {
  return json{{"player_id", player.play_id()},                 // 玩家id
              {"alive", player.status() == PlayStatus::ALIVE}, // 是否存活
              {"player_name", player.player_name()},           // 玩家名字
              {"hp", player.HP()},                             // 血量
              {"shield_time", player.shield_time()}, // 护盾剩余回合数
              {"invincible_time", player.invincible_time()}, // 无敌回合数
              {"score", player.mark()},                      // 当前总分数
              {"bomb_range", player.bomb_range()},     // 炸弹爆炸范围
              {"bomb_max_num", player.bomb_max_num()}, // 炸弹数量上限
              {"bomb_now_num", player.bomb_now_num()}, // 现在放了的炸弹数量
              {"has_gloves", player.has_gloves()}, // 是否拥有推炸弹技能
              {"speed", player.speed()}};          // 当前剩余炸弹
}

json customed_to_json(const ::Bomb &bomb) {
  return json{{"bomb_id", bomb.bomb_id()},
              {"bomb_status", bomb.bomb_status()},
              {"bomb_range", bomb.bomb_range()},
              {"player_id", bomb.player_id()}};
}

json customed_to_json(const ::BlockBase &block) {
  return json{{"block_id", block.block_id()},
              {"removable", block.IsBombAble()}};
}

json customed_to_json(::Area &area) {
  Game &game = Game::GetInstance();

  auto j = json{{"x", area.pos().first},
                {"y", area.pos().second},
                {"last_bomb_round", area.last_bomb_round()}};

  auto gen_obj = [](ObjType type, json &&data) {
    return json{{"type", type}, {"property", data}};
  };

  j["objs"] = json::array();

  // add players
  for (auto player_id : area.players()) {
    auto player_ptr = game.GetPlayerByID(player_id);
    j["objs"].push_back(
        gen_obj(ObjType::PLAYER, std::move(customed_to_json(*player_ptr))));
  }

  // add bomb
  if (area.bomb_id() != -1) {
    auto bomb_ptr = game.GetBombByID(area.bomb_id());
    j["objs"].push_back(
        gen_obj(ObjType::BOMB, std::move(customed_to_json(*bomb_ptr))));
  }

  // add block
  if (area.block_id() != -1) {
    auto block_ptr = game.GetBlockByID(area.block_id());
    j["objs"].push_back(
        gen_obj(ObjType::BLOCK, std::move(customed_to_json(*block_ptr))));
  }

  // add item
  if (area.potion_type() != PotionType::NO_POTION) {
    j["objs"].push_back(gen_obj(
        ObjType::ITEM, std::move(json{{"item_type", area.potion_type()}})));
  }
  return j;
}

json customed_to_json(
    const std::vector<std::vector<std::shared_ptr<Area>>> &area_map) {
  auto j = json{};
  for (int x = 0; x < area_map.size(); ++x) {
    for (int y = 0; y < area_map[0].size(); ++y) {
      j.push_back(customed_to_json(*area_map[x][y]));
    }
  }
  return j;
}

// nyw add
int handle_timer_intr(std::map<int, std::string> &fd2msg,
                      std::map<int, int> &fd2PlayerId) {
  Game &game = Game::GetInstance();

  if (game.game_status() != GameStatus::WAIT_ACTION) {
    return false;
  }

  std::vector<ID> winner_ids;
  auto result = game.FlushTime(winner_ids);
  if (result != RC::SUCCESS) {
    return false;
  }

  // send data back to client
  auto players =
      std::vector<std::pair<int, int>>(fd2PlayerId.begin(), fd2PlayerId.end());
  std::random_shuffle(players.begin(), players.end());

  enum RespType {
    Action = 3,
    GameOver,
  };

  enum ResultType { WIN = 0, LOSS, TIE };

  auto gen_result = [](int type, json &&data) {
    return json{{"type", type}, {"data", data}};
  };

  if (game.game_status() == GameStatus::GAME_OVER) {
    auto result = json{{"scores", json{}}, {"winner_ids", json::array()}};
    auto json_file = json{};

    for (auto &[fd, player_id] : players) {
      auto player_ptr = game.GetPlayerByID(player_id);
      result["scores"].push_back(
          json{{"player_id", player_id}, {"score", player_ptr->mark()}});

      json_file["scores"].push_back(
          json{{"player_name", player_ptr->player_name()},
               {"score", player_ptr->mark()}});
    }

    for (auto &id : winner_ids) {
      result["winner_ids"].push_back(id);
    }

    result = gen_result(RespType::GameOver, std::move(result));

    for (auto &[fd, player_id] : players) {
      fd2msg[fd] = result.dump();
    }

    for (auto &id : winner_ids) {
      json_file["winners"].push_back(
          json{{"player_name", game.GetPlayerByID(id)->player_name()},
               {"score", game.GetPlayerByID(id)->mark()}});
    }

    std::ofstream output_file;
    if (kResultPath == "") {
      try {
        kResultPath = Config::get_instance().get<std::string>("result_path");
      } catch (std::exception) {
        kResultPath = "./result.json";
      }
    }
    output_file.open(kResultPath, std::ios::out | std::ios::trunc);
    output_file << json_file.dump();

    output_file.close();
    return true;
  }

  for (auto &[fd, player_id] : players) {
    auto player_ptr = game.GetPlayerByID(player_id);
    auto resp = json{{"player_id", player_ptr->play_id()},
                     {"round", game.game_now_round()}};
    resp["map"] = customed_to_json(game.map());
    fd2msg[fd] = gen_result(RespType::Action, std::move(resp)).dump();
  }
  return false;
}

bool game_reset() {
  Game &game = Game::GetInstance();
  int retval = (game.Reset() == RC::SUCCESS);
  retval = game.Init();
  return retval == RC::SUCCESS;
}

} // namespace API
