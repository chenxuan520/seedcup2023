#pragma once

#include "potion_base.h"
class ShieldPotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_shield_time(kShieldDefaultTime);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return SHIELD; }
};
