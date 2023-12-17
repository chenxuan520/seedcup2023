#pragma once

#include "potion_base.h"
class GlovesPotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_has_gloves(true);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return INVINCIBLE; }
};
