#pragma once
#include "../game/action.h" // nyw include for ActionType
#include "../game/const.h"
#include "../game/game.h"
#include <json.hpp>
#include <unordered_set>
#include <vector>

using namespace nlohmann;

namespace API {

enum PacketType { Init = 1, Action };
enum ObjType { PLAYER = 1, BOMB, BLOCK, ITEM };

extern std::unordered_set<std::string> kBlackList;

class InitReq {
public:
  InitReq() = default; // NOTE(nyw):InitReq没有参数,有的话再加
};

class ActionReq {
public:
  ActionReq() = default;
  ActionReq(int playerID, ActionType action_type)
      : playerID(playerID), action_type(action_type) {}

  int playerID;
  ActionType action_type;
};

void to_json(json &j, const InitReq &req);
void to_json(json &j, const ActionReq &req);
json customed_to_json(::Area &area);
json customed_to_json(::Player &player);
json customed_to_json(const ::Bomb &bomb);
json customed_to_json(const ::BlockBase &block);
json customed_to_json(
    std::vector<std::vector<std::shared_ptr<Area>>> &area_map);
void handle_init(int &player_id, const std::string &player_name);
void from_json(const json &j, InitReq &req);
void from_json(const json &j, ActionReq &req);

int handle_timer_intr(
    std::map<int, std::string> &fd2msg,
    std::map<int, int> &fd2PlayerId); // NOTE(nyw): this func would be used as a
                                      // callback func when a round ends
void handle_request(json req, int &player_id);
void handle_init(int &player_id, const std::string &player_name);
void handle_action(const json &data, const int &player_id);
bool game_reset();
} // namespace API
