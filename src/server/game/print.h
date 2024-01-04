#pragma once
#include "game.h"
#include <memory>
#include <unordered_map>
#include <vector>
#define PRINTOUT(text) std::cout << text;
#define PRINTERR(text) std::cerr << text;

// print kinds of color
#define PRINTRED(text) "\033[31m" << text << "\033[0m"
#define PRINTGREEN(text) "\033[32m" << text << "\033[0m"
#define PRINTYELLOW(text) "\033[33m" << text << "\033[0m"
#define PRINTBLUE(text) "\033[34m" << text << "\033[0m"
#define PRINTCAR(text) "\033[35m" << text << "\033[0m"
#define PRINTCYAN(text) "\033[36m" << text << "\033[0m"

class Print {
public:
  enum IconType { NONE, NORMAL_ICON, ASCII_ICON, CUSTOM_ICON };

public:
  static Print &GetInstance() {
    static Print instance;
    return instance;
  }

  void PrintConst() {
    PRINTOUT("CONST:"
             << "\nbomb_time:" << kBombDefaultTime << "\nbomb_range:"
             << kBombDefaultRange << "\nbomb_num:" << kBombDefaultNum
             << "\nbomb_random:" << kBombDefaultRandom << "\nmap_size:"
             << kMapDefaultSize << "\nplayer_num:" << kPlayerDefaultNum
             << "\nplayer_hp:" << kPlayerDefaultHP << "\nplayer_max_hp:"
             << kPlayerDefaultMaxHP << "\nplayer_speed:" << kPlayerDefaultSpeed
             << "\nshield_time:" << kShieldDefaultTime
             << "\nmark_kill:" << kMarkKill << "\nmark_dead:" << KMarkDead
             << "\nmark_pick_potion:" << kMarkPick << "\nmark_bomb_mud:"
             << kMarkBombMud << "\ngame_max_round:" << kGameDefaultMaxRound
             << "\nseed_random:" << kSeedRandom
             << "\npotion_probability:" << kPotionDefaultProbability
             << "\ninvincible_time:" << kInvisibleDefaultTime
             << "\nmud_num:" << kMudDefaultNum
             << "\nwall_random:" << kWallDefaultRandom << std::endl);
  }

  void PrintSnapshot(const std::vector<std::vector<std::shared_ptr<Area>>> &map,
                     int now_round) {
    for (int i = 0; i < map.size() + 2; i++) {
      PRINTOUT(boundary_to_print_);
    }
    PRINTOUT(std::endl);
    for (int i = 0; i < map.size(); i++) {
      PRINTOUT(boundary_to_print_);
      for (int j = 0; j < map[i].size(); j++) {
        auto &area = map[i][j];
        if (area->players().size() != 0) {
          auto iter = area->players().begin();
          auto play_id = *iter;
          PRINTOUT(player_to_print_[play_id % 4]);

        } else if (area->bomb_id() != -1) {
          auto bomb_id = area->bomb_id();
          PRINTOUT(PRINTYELLOW(bomb_to_print_));

        } else if (area->block_id() != -1) {
          auto blob_id = area->block_id();
          PRINTOUT(PRINTGREEN(
              block_to_print_[(blob_id + diff) % block_to_print_.size()]));

        } else if (area->potion_type() != NO_POTION) {
          PRINTOUT(PRINTCAR(potion_to_print_[area->potion_type()]));

        } else {
          // 普通土地
          PRINTOUT(empty_to_print_);
        }
      }
      PRINTOUT(boundary_to_print_);
      PRINTOUT(std::endl);
    }
    for (int i = 0; i < map.size() + 2; i++) {
      PRINTOUT(boundary_to_print_);
    }
    PRINTOUT(std::endl);
    PRINTOUT(PRINTBLUE("🎮game_round:" << now_round << std::endl));
  }

  void PrintMap(Game &game) {

    PRINTOUT(std::endl);
    for (auto iter : game.bomb_map()) {
      auto bomb = iter.second;
      PRINTOUT(PRINTCYAN("💣bomb_pos:" << bomb->pos().first << ","
                                      << bomb->pos().second << " 👤owner_id:"
                                      << bomb->player_id() << " 🕤bomb_time:"
                                      << bomb->bomb_time() << " 🧪bomb_range:"
                                      << bomb->bomb_range() << std::endl));
    }
    PRINTOUT(std::endl);
    for (auto iter : game.player_map()) {
      std::string begin, end = "\033[0m";
      auto player = iter.second;
      if (player->status() == ALIVE) {
        begin = "\033[32m";
      } else {
        begin = "\033[31m";
      }
      PRINTOUT(begin << "👤player_name:" << player->player_name() << " "
                     << player_to_print_[player->play_id() % 4] << "player_id:"
                     << player->play_id() << " 💖HP:" << player->HP()
                     << " 🗽invincible_time:" << player->invincible_time()
                     << " 🔰shield_time:" << player->shield_time()
                     << " 🧪bomb_range:" << player->bomb_range()
                     << " 💊bomb_max_num:" << player->bomb_max_num()
                     << " 💣bomb_now_num:" << player->bomb_now_num()
                     << " 🧤has_gloves:" << player->has_gloves() << " 🚅speed:"
                     << player->speed() << " 💯mark:" << player->mark()
                     << " 💺pos:" << player->pos().first << ","
                     << player->pos().second << end << std::endl);
    }
    PRINTOUT(
        PRINTYELLOW("🎮game_round:" << game.game_now_round() << " 🟫block_num:"
                                   << game.block_map().size() << std::endl));
    for (int i = 0; i < kMapDefaultSize + 2; i++) {
      PRINTOUT(boundary_to_print_);
    }
    PRINTOUT(std::endl);
    for (int i = 0; i < kMapDefaultSize; i++) {
      PRINTOUT(boundary_to_print_);
      for (int j = 0; j < kMapDefaultSize; j++) {
        auto &area = game.map()[i][j];
        if (area->players().size() != 0) {
          auto iter = area->players().begin();
          auto play_id = *iter;
          auto player = game.GetPlayerByID(play_id);
          if (player->invincible_time() > 0) {
            PRINTOUT(invincible_to_print_);
          } else if (player->shield_time() > 0) {
            PRINTOUT(shield_to_print_);
          } else {
            PRINTOUT(player_to_print_[play_id % 4]);
          }

        } else if (area->bomb_id() != -1) {
          auto bomb_id = area->bomb_id();
          PRINTOUT(PRINTYELLOW(bomb_to_print_));

        } else if (area->block_id() != -1) {
          auto blob_id = area->block_id();
          auto block = game.GetBlockByID(blob_id);
          if (block->IsBombAble()) {
            PRINTOUT(PRINTGREEN(
                block_to_print_[(blob_id + diff) % block_to_print_.size()]));
          } else {
            PRINTOUT(wall_to_print_);
          }

        } else if (area->potion_type() != NO_POTION) {
          PRINTOUT(PRINTCAR(potion_to_print_[area->potion_type()]));

        } else {
          if (area->last_bomb_round() == game.game_now_round()) {
            // 被炸土地
            PRINTOUT(effect_to_print_);
          } else {
            // 普通土地
            PRINTOUT(empty_to_print_);
          }
        }
      }
      PRINTOUT(boundary_to_print_);
      PRINTOUT(std::endl);
    }
    for (int i = 0; i < kMapDefaultSize + 2; i++) {
      PRINTOUT(boundary_to_print_);
    }
    PRINTOUT(std::endl);
    PRINTOUT("PotionType:" << potion_to_print_[BOMB_RANGE] << ":BOMB_RANGE "
                           << potion_to_print_[BOMB_NUM] << ":BOMB_NUM "
                           << potion_to_print_[INVINCIBLE] << ":INVINCIBLE "
                           << potion_to_print_[SHIELD] << ":SHIELD "
                           << potion_to_print_[REBIRTH] << ":REBIRTH "
                           << potion_to_print_[SPEED] << ":SPEED" << std::endl);
  }

  inline const std::unordered_map<PotionType, std::string> &
  potion_to_print() const {
    return potion_to_print_;
  }
  inline const std::unordered_map<ID, std::string> &player_to_print() const {
    return player_to_print_;
  }
  inline const std::vector<std::string> &block_to_print() const {
    return block_to_print_;
  }
  inline const std::string &boundary_to_print() const {
    return boundary_to_print_;
  }
  inline const std::string &empty_to_print() const { return empty_to_print_; }
  inline const std::string &wall_to_print() const { return wall_to_print_; }
  inline const std::string &bomb_to_print() const { return bomb_to_print_; }

private:
  IconType icon_type_ = NORMAL_ICON;
  std::unordered_map<PotionType, std::string> potion_to_print_ = {
      {BOMB_RANGE, "🧪"}, {BOMB_NUM, "💊"}, {INVINCIBLE, "🗽"}, {SHIELD, "🔰"},
      {SPEED, "🚅"},         {REBIRTH, "💖"},  {GLOVES, "🧤"}};
  std::unordered_map<ID, std::string> player_to_print_ = {
      {0, "0️⃣ "},
      {1, "1️⃣ "},
      {2, "2️⃣ "},
      {3, "3️⃣ "},
  };

  std::vector<std::string> block_to_print_ = {"🟧", "🟫", "🟦",
                                              "🟩", "🟥", "🟪"};
  int diff = std::rand() % 3;
  std::string wall_to_print_ = "🧱";
  std::string bomb_to_print_ = "💣";
  std::string invincible_to_print_ = "👼";
  std::string shield_to_print_ = "👒";
  std::string effect_to_print_ = "💥";
  std::string boundary_to_print_ = "💮";
  std::string empty_to_print_ = "◻️ ";

private:
  // 单例模式
  Print() {
    // 统一第一次进入修改
    if (kIsExistCustomIcon) {
      icon_type_ = CUSTOM_ICON;

      const auto &icon_config =
          Config::get_instance().get_json()["custom_icon"];
      potion_to_print_[BOMB_RANGE] = potion_to_print_[BOMB_NUM] =
          icon_config["bomb_range_item_icon"];
      potion_to_print_[INVINCIBLE] = icon_config["invincible_item_icon"];
      potion_to_print_[SHIELD] = icon_config["shield_item_icon"];
      potion_to_print_[REBIRTH] = icon_config["hp_item_icon"];
      potion_to_print_[SPEED] = icon_config["speed_item_icon"];
      potion_to_print_[GLOVES] = icon_config["gloves_item_icon"];
      if (icon_config.contains("player_icon") &&
          icon_config["player_icon"].is_array()) {
        for (int i = 0; i < std::min(4, (int)icon_config["player_icon"].size());
             i++) {
          player_to_print_[i] = icon_config["player_icon"][i];
        }
      }
      block_to_print_ = icon_config["block_icon"];
      wall_to_print_ = icon_config["wall_icon"];
      bomb_to_print_ = icon_config["bomb_icon"];
      invincible_to_print_ = icon_config["invincible_player_icon"];
      shield_to_print_ = icon_config["shield_player_icon"];
      effect_to_print_ = icon_config["bomb_effect_icon"];
      boundary_to_print_ = icon_config["boundary_icon"];
      empty_to_print_ = icon_config["empty_area_icon"];

    } else if (kIsGamePrintMapAscii) {
      icon_type_ = ASCII_ICON;

      potion_to_print_[BOMB_NUM] = "a ";
      potion_to_print_[BOMB_RANGE] = "b ";
      potion_to_print_[INVINCIBLE] = "c ";
      potion_to_print_[REBIRTH] = "d ";
      potion_to_print_[SHIELD] = "e ";
      potion_to_print_[SPEED] = "f ";
      potion_to_print_[GLOVES] = "g ";
      player_to_print_[0] = "0 ";
      player_to_print_[1] = "1 ";
      player_to_print_[2] = "2 ";
      player_to_print_[3] = "3 ";
      block_to_print_ = std::vector<std::string>(1, "6 ");
      wall_to_print_ = "9 ";
      bomb_to_print_ = "8 ";
      invincible_to_print_ = "$ ";
      shield_to_print_ = "% ";
      effect_to_print_ = "X ";
      boundary_to_print_ = "";
      empty_to_print_ = "_ ";

    } else {
      icon_type_ = NORMAL_ICON;
    }
  }

  Print(const Print &) = delete;
  Print &operator=(const Print &) = delete;
};
