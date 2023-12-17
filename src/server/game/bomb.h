#pragma once
#include "config.h"
#include "const.h"
#include "random.h"
#include <cstdlib>
#include <memory>
#include <unordered_map>

enum BombStatus {
  BOMB_SILENT,
  BOMB_MOVE_LEFT,
  BOMB_MOVE_RIGHT,
  BOMB_MOVE_UP,
  BOMB_MOVE_DOWN,
};

class Bomb final {
public:
  Bomb(Pos pos, int range, ID player_id)
      : pos_(pos), bomb_range_(range), player_id_(player_id) {
    static int bomb_id = 0;
    static Random rand_creater(0, kBombDefaultRandom + 1, kSeedRandom);
    bomb_id_ = bomb_id++;
    bomb_id %= kIncrMaxID;
    bomb_time_ += rand_creater.CreateRandom();
  }

  bool FlushTime() {
    if (bomb_time_ > 0) {
      bomb_time_--;
      return false;
    }
    return true;
  }

  RC set_bomb_time(int bomb_time) {
    ASSERT(bomb_time >= 0);
    bomb_time_ = bomb_time;
    return RC::SUCCESS;
  }

  RC set_bomb_status(BombStatus bomb_status) {
    bomb_status_ = bomb_status;
    return RC::SUCCESS;
  }

  RC set_pos(Pos pos) {
    pos_ = pos;
    return RC::SUCCESS;
  }

  inline int bomb_range() const { return bomb_range_; }
  inline int bomb_time() const { return bomb_time_; }
  inline int player_id() const { return player_id_; }
  inline ID bomb_id() const { return bomb_id_; }
  inline Pos pos() const { return pos_; }
  inline BombStatus bomb_status() const { return bomb_status_; }

private:
  ID bomb_id_ = -1;
  Pos pos_{-1, -1};
  int bomb_time_ = kBombDefaultTime;     // 爆炸时间
  int bomb_range_ = 1;                   // 范围
  int player_id_ = -1;                   // 放置人
  BombStatus bomb_status_ = BOMB_SILENT; // 炸弹运动状态
};
