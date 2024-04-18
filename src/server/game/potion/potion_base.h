#pragma once
#include "../player.h"
#include "random.h"
#include <memory>
#include <unordered_map>
#include <vector>

enum PotionType {
  NO_POTION = 0,
  BOMB_RANGE,
  BOMB_NUM,
  REBIRTH,
  INVINCIBLE,
  SHIELD,
  SPEED,
  GLOVES,
};

class PotionBase {
public:
  virtual Mark PickUp(std::shared_ptr<Player> player) = 0;
  virtual PotionType GetPotionType() = 0;
  static PotionType GenRandomPotion() {
    // 调整这个数组的比例动态调整生成的药水类型
    static std::vector<PotionType> random_potion = {
        SHIELD,     SHIELD,     INVINCIBLE, REBIRTH,    REBIRTH,    BOMB_RANGE,
        BOMB_RANGE, BOMB_RANGE, BOMB_RANGE, BOMB_RANGE, BOMB_RANGE, BOMB_NUM,
        BOMB_NUM,   BOMB_NUM,   BOMB_NUM,   BOMB_NUM,   BOMB_NUM,   SPEED,
        SPEED,      SPEED,      GLOVES,     GLOVES,     GLOVES,
    };
    static Random rand_creater(0, random_potion.size(), kSeedRandom);
    return random_potion[rand_creater.CreateRandom()];
  }
};
