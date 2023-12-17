#pragma once
#include "potion_base.h"

class RangePotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_bomb_range(player->bomb_range() + 1);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return BOMB_RANGE; }
};
