#pragma once
#include "../player.h"
#include "random.h"
#include <algorithm>
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
  static const std::vector<PotionType> &PotionList() {
    static const std::shared_ptr<std::vector<PotionType>> random_potion =
        GenPotionList();
    return *random_potion;
  }
  static PotionType GenRandomPotion() {
    const auto &potion_list = PotionList();
    static Random rand_creater(0, potion_list.size(), kSeedRandom);
    return potion_list[rand_creater.CreateRandom()];
  }

private:
  static std::shared_ptr<std::vector<PotionType>> GenPotionList() {
    std::vector<PotionType> potion_list;
    constexpr size_t potion_type_count = static_cast<size_t>(GLOVES);
    if (kPotionDefaultProbabilityList.size() == potion_type_count) {
      for (size_t index = 0; index < potion_type_count; ++index) {
        const int weight = std::max(kPotionDefaultProbabilityList[index], 0);
        potion_list.insert(potion_list.end(), weight,
                           static_cast<PotionType>(index + 1));
      }
    }

    if (potion_list.empty()) {
      // 普通道具保持 6:6:2:2:3:3，并统一扩大两倍；无敌保持 1。
      const int default_weights[] = {12, 12, 4, 1, 4, 6, 6};
      for (size_t index = 0; index < potion_type_count; ++index) {
        potion_list.insert(potion_list.end(), default_weights[index],
                           static_cast<PotionType>(index + 1));
      }
    }
    return std::make_shared<std::vector<PotionType>>(potion_list);
  }
};
