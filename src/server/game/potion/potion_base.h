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
    static std::shared_ptr<std::vector<PotionType>> random_potion =
        GenPotionList();
    static Random rand_creater(0, random_potion->size(), kSeedRandom);
    return (*random_potion)[rand_creater.CreateRandom()];
  }

private:
  static std::shared_ptr<std::vector<PotionType>> GenPotionList() {
    std::vector<PotionType> potion_list = {};
    std::underlying_type<PotionType>::type potion_max_type =
        std::numeric_limits<std::underlying_type<PotionType>::type>::max();
    if (kPotionDefaultProbabilityList.size() != potion_max_type - 1) {
      // 生成默认比例的药水类型 5:5:2:1:2:3:3
      potion_list = {
          SHIELD,     SHIELD,     INVINCIBLE, REBIRTH,    REBIRTH,
          BOMB_RANGE, BOMB_RANGE, BOMB_RANGE, BOMB_RANGE, BOMB_RANGE,
          BOMB_RANGE, BOMB_NUM,   BOMB_NUM,   BOMB_NUM,   BOMB_NUM,
          BOMB_NUM,   BOMB_NUM,   SPEED,      SPEED,      SPEED,
          GLOVES,     GLOVES,     GLOVES,
      };
    } else {
      for (int i = 1; i < kPotionDefaultProbabilityList.size(); i++) {
        potion_list.push_back(PotionType(i));
      }
    }
    return std::make_shared<std::vector<PotionType>>(potion_list);
  }
};
