#pragma once

#include "config.h"
#include <assert.h>
#include <cmath>
#include <ctime>
#include <math.h>
#include <memory>
#include <utility>

#ifndef RELEASE
#define ASSERT(flag) assert(flag)
#else
#define ASSERT(flag) (void *(0))
#endif

using Pos = std::pair<int, int>;
using ID = int;
using Mark = int;

const int kBombDefaultTime =
    std::max(Config::get_instance().get<int>("bomb_time"), 3);
const int kBombDefaultRange =
    std::max(Config::get_instance().get<int>("bomb_range"), 1);
const int kBombDefaultNum =
    std::max(Config::get_instance().get<int>("bomb_num"), 1);
const int kBombDefaultRandom =
    std::max(Config::get_instance().get<int>("bomb_random"), 0);
const int kMapDefaultSize = std::max(
    std::min(21, 2 * (Config::get_instance().get<int>("map_size") / 2) + 1), 5);
const int kPlayerDefaultNum =
    std::max(std::min(4, Config::get_instance().get<int>("player_num")), 2);
const int kPlayerDefaultHP =
    std::max(Config::get_instance().get<int>("player_hp"), 1);
const int kPlayerDefaultMaxHP =
    std::max(Config::get_instance().get<int>("player_max_hp"), 1);
const int kPlayerDefaultSpeed =
    std::max(Config::get_instance().get<int>("player_speed"), 1);
const int kShieldDefaultTime =
    std::max(Config::get_instance().get<int>("shield_time"), 1);
const int kMarkKill = std::max(Config::get_instance().get<int>("mark_kill"), 1);
const int KMarkDead = std::max(Config::get_instance().get<int>("mark_dead"), 1);
const int kMarkPick =
    std::max(Config::get_instance().get<int>("mark_pick_potion"), 1);
const int kMarkBombMud =
    std::max(Config::get_instance().get<int>("mark_bomb_mud"), 1);
const int kGameDefaultMaxRound =
    std::max(Config::get_instance().get<int>("game_max_round"), 100);
const int kSeedRandom = Config::get_instance().get<int>("seed_random") != 0
                            ? Config::get_instance().get<int>("seed_random")
                            : time(0);
const int kPotionDefaultProbability = std::min(
    std::max(Config::get_instance().get<int>("potion_probability"), 0), 100);
const int kInvisibleDefaultTime =
    std::max(Config::get_instance().get<int>("invincible_time"), 1);
const int kMudDefaultNum = std::min(
    std::max(Config::get_instance().get<int>("mud_num"), 0),
    (int)pow(kMapDefaultSize, 2) - (int)pow((kMapDefaultSize - 1) / 2, 2) - 12);
const int kWallDefaultRandom =
    std::min(std::max(Config::get_instance().get<int>("wall_random"), 0), 100);
const bool kIsGamePrintMap = Config::get_instance().get<bool>("game_print_map");
const bool kIsGamePrintMapAscii =
    Config::get_instance().get<bool>("game_print_map_ascii");
const bool kIsLogPrintStdout =
    Config::get_instance().get<bool>("log_print_stdout");
const bool kIsSnapshot = Config::get_instance().get<bool>("game_snapshot");
const int kIncrMaxID = 10000000;
const bool kIsExistCustomIcon =
    Config::get_instance().get_json().contains("custom_icon");
const bool kIsExistCustomMap =
    Config::get_instance().get_json().contains("custom_map");

enum RC {
  SUCCESS = 0,
  MOVE_OUT_MAP,       // 移动出地图
  MOVE_NOT_ALLOW,     // 无法移动,可能遇到墙壁了
  PLAYER_DEAD,        // 用户死亡,无法操作
  PLAYER_TOO_MUCH,    // 用户数量到达无法添加
  ACTION_TOO_MUCH,    // 一回合操作次数超过移速
  BOMB_TOO_MUCH,      // 到达放置炸弹的上限
  BOMB_NO_ALLOW,      // 无法放置,可能是地上有炸弹了
  INVALUE_OPER,       // 无效的操作,当前状态无法操作
  INVALUS_CUSTOM_MAP, // 无效的自定义地图
};
