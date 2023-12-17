#pragma once
#include "gloves_potion.h"
#include "invincible_potion.h"
#include "num_potion.h"
#include "range_potion.h"
#include "rebirth_potion.h"
#include "shield_potion.h"
#include "speed_potion.h"
#include <memory>

class PotionFactory {
public:
  static std::shared_ptr<PotionBase> GenPotion(PotionType potion_type) {
    switch (potion_type) {
    case BOMB_RANGE:
      return std::make_shared<RangePotion>();
    case BOMB_NUM:
      return std::make_shared<NumPotion>();
    case REBIRTH:
      return std::make_shared<RebirthPotion>();
    case INVINCIBLE:
      return std::make_shared<InvinciblePotion>();
    case SHIELD:
      return std::make_shared<ShieldPotion>();
    case SPEED:
      return std::make_shared<SpeedPotion>();
    case GLOVES:
      return std::make_shared<GlovesPotion>();
    case NO_POTION:
      return nullptr;
    }
    return nullptr;
  }
};
