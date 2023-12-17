#pragma once

#include "potion_base.h"
class SpeedPotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_speed(player->speed() + 1);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return SPEED; }
};
