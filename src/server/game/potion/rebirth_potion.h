#pragma once

#include "potion_base.h"
class RebirthPotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->IncrHP();
    return kMarkPick;
  }
  PotionType GetPotionType() override { return REBIRTH; }
};
