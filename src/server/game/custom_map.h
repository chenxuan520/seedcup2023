#pragma once
#include "block/block_base.h"
#include "config.h"
#include "const.h"
#include "potion/potion_base.h"
#include <unordered_map>
#include <vector>

class CustomMap final {
public:
  CustomMap(const CustomMap &) = delete;
  CustomMap &operator=(const CustomMap &) = delete;
  static CustomMap &GetInstance() {
    static CustomMap custom_map;
    return custom_map;
  }
  int GetCustomMap(std::vector<std::vector<std::string>> &map) {
    auto &config_json = Config::get_instance().get_json();
    if (!kIsExistCustomMap) {
      error = "custom map not exist";
      return -1;
    }
    auto &custom_map_json = config_json["custom_map"];
    for (auto &row : custom_map_json) {
      std::vector<std::string> row_vec;
      for (auto &col : row) {
        row_vec.push_back(col);
      }
      map.push_back(row_vec);
    }
    return 0;
  }

  CustomMap() {
    // 这里和print那部分重合了,TODO
    potion_type_map_ = {
        {"a", PotionType::BOMB_NUM},   {"b", PotionType::BOMB_RANGE},
        {"c", PotionType::INVINCIBLE}, {"d", PotionType::REBIRTH},
        {"e", PotionType::SHIELD},     {"f", PotionType::SPEED},
        {"g", PotionType::GLOVES},
    };
    block_type_map_ = {
        {"6", BlockType::MUD},
        {"9", BlockType::WALL},
    };
    player_print_ = {"0", "1", "2", "3"};
    empty_print_ = "_";
  }
  inline int GetPlayerPrintByID(ID player_id, std::string &print) {
    print = player_print_[player_id % 4];
    return 0;
  }
  inline bool IsEmptyPrint(const std::string &print) {
    return print == empty_print_;
  }
  inline bool IsPotionPrint(const std::string &print) {
    return potion_type_map_.find(print) != potion_type_map_.end();
  }
  inline bool IsBlockPrint(const std::string &print) {
    return block_type_map_.find(print) != block_type_map_.end();
  }
  inline bool IsPlayerPrint(const std::string &print) {
    for (auto &player_print : player_print_) {
      if (player_print == print) {
        return true;
      }
    }
    return false;
  }
  inline BlockType GetBlockTypeByPrint(const std::string &print) {
    return block_type_map_[print];
  }
  inline PotionType GetPotionTypeByPrint(const std::string &print) {
    return potion_type_map_[print];
  }
  inline int GetPlayerIDByPrint(const std::string &print) {
    for (int i = 0; i < player_print_.size(); i++) {
      if (player_print_[i] == print) {
        return i;
      }
    }
    error = "print no found";
    return -1;
  }
  inline std::string GetLastError() { return error; }

private:
  std::unordered_map<std::string, PotionType> potion_type_map_;

  std::unordered_map<std::string, BlockType> block_type_map_;

  std::vector<std::string> player_print_;
  std::string empty_print_;
  std::string error;
};
