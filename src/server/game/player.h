#pragma once
#include "config.h"
#include "const.h"
#include "unordered_map"
#include <memory>
#include <string>
enum PlayStatus {
  ALIVE = 0,
  DEAD,
};

class Player {
public:
  Player(Pos pos, const std::string &player_name) : pos_(pos) {
    static int play_id = 0;
    play_id_ = play_id++;
    play_id %= kIncrMaxID;
    player_name_ = player_name;
  }

  RC set_pos(Pos new_pos) {
    if (new_pos.first < 0 || new_pos.first >= kMapDefaultSize ||
        new_pos.second < 0 || new_pos.second >= kMapDefaultSize) {
      return RC::MOVE_OUT_MAP;
    }
    pos_ = new_pos;
    return RC::SUCCESS;
  }

  RC IncrMark(Mark mark_add) {
    mark_ += mark_add;
    return RC::SUCCESS;
  }

  RC DescMark(Mark mark_desc) {
    mark_ -= mark_desc;
    return RC::SUCCESS;
  }

  void IncrHP() {
    if (status_ == DEAD) {
      return;
    }
    if (HP_ + 1 <= kPlayerDefaultMaxHP) {
      HP_++;
    }
  }

  RC set_invincible_time(int invincible_time) {
    ASSERT(invincible_time > 0);
    invincible_time_ = invincible_time;
    return RC::SUCCESS;
  }

  RC set_mark(Mark mark) {
    mark_ = mark;
    return RC::SUCCESS;
  }

  RC set_shield_time(int shield_time) {
    ASSERT(invincible_time_ >= 0);
    shield_time_ = shield_time;
    return RC::SUCCESS;
  }

  RC set_bomb_range(int bomb_range) {
    ASSERT(bomb_range > 0);
    bomb_range_ = bomb_range;
    return RC::SUCCESS;
  }

  RC set_bomb_max_num(int bomb_max_num) {
    ASSERT(bomb_max_num > 0);
    bomb_max_num_ = bomb_max_num;
    return RC::SUCCESS;
  }

  RC set_bomb_now_num(int bomb_now_num) {
    ASSERT(bomb_now_num <= bomb_max_num_);
    bomb_now_num_ = bomb_now_num;
    return RC::SUCCESS;
  }

  RC set_speed(int speed) {
    ASSERT(speed > 0);
    speed_ = speed;
    return RC::SUCCESS;
  }

  RC set_has_gloves(bool has_gloves) {
    has_gloves_ = has_gloves;
    return RC::SUCCESS;
  }

  void FlushTime() {
    if (shield_time_ > 0) {
      shield_time_--;
    }
    if (invincible_time_ > 0) {
      invincible_time_--;
    }
  }

  Mark Injuries() {
    if (status_ == DEAD) {
      return 0;
    }
    if (invincible_time_ > 0) {
      return 0;
    }
    if (shield_time_ > 0) {
      shield_time_ = 0;
      return 0;
    }
    return DescHP();
  }

  Mark MeetOtherPlayer(std::shared_ptr<Player> other_player) {
    if (other_player == nullptr || other_player->play_id() == play_id_) {
      return 0;
    }
    // 两个人都无敌
    if (other_player->invincible_time() > 0 && this->invincible_time_ > 0) {
      return 0;
    }
    // 对手无敌
    if (other_player->invincible_time() > 0) {
      // 自己受伤并加别人积分
      auto mark_add = Injuries();
      other_player->IncrMark(mark_add);
      return -mark_add;
    }
    if (this->invincible_time() > 0) {
      // 自己无敌
      auto mark_add = other_player->Injuries();
      this->IncrMark(mark_add);
      return mark_add;
    }
    return 0;
  }

  inline ID play_id() const { return play_id_; }
  inline Pos pos() const { return pos_; }
  inline PlayStatus status() const { return status_; }
  inline int HP() const { return HP_; }
  inline int shield_time() const { return shield_time_; }
  inline int invincible_time() const { return invincible_time_; }
  inline int mark() const { return mark_; }
  inline int bomb_range() const { return bomb_range_; }
  inline int bomb_max_num() const { return bomb_max_num_; }
  inline int bomb_now_num() const { return bomb_now_num_; }
  inline int speed() const { return speed_; }
  inline bool has_gloves() const { return has_gloves_; }
  inline const std::string &player_name() { return player_name_; }

private:
  Mark DescHP() {
    HP_--;
    if (HP_ <= 0) {
      status_ = DEAD;
      DescMark(kMarkDead);
      return kMarkKill;
    }
    this->shield_time_ = kShieldDefaultTime;
    return 0;
  }

private:
  ID play_id_ = -1;                    // 玩家id
  std::string player_name_ = "";       // 玩家名字
  Pos pos_{-1, -1};                    // 位置
  PlayStatus status_ = ALIVE;          // 目前状态
  int HP_ = kPlayerDefaultHP;          // 血量
  int shield_time_ = 0;                // 护盾时间剩余
  int invincible_time_ = 0;            // 无敌时间
  int speed_ = kPlayerDefaultSpeed;    // 移动速度
  Mark mark_ = 0;                      // 积分
  int bomb_range_ = 1;                 // 炸弹范围
  int bomb_max_num_ = kBombDefaultNum; // 最大同时炸弹数量
  int bomb_now_num_ = 0;               // 目前已经放置的炸弹数量
  bool has_gloves_ = false;            // 是否获取手套技能
};
