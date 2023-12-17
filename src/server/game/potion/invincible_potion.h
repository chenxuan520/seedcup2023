#pragma once

#include "potion_base.h"
class InvinciblePotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_invincible_time(kInvisibleDefaultTime);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return INVINCIBLE; }
};
