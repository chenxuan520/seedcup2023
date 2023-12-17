#pragma once

#include "potion_base.h"

class NumPotion : public PotionBase {
public:
  Mark PickUp(std::shared_ptr<Player> player) override {
    player->set_bomb_max_num(player->bomb_max_num() + 1);
    return kMarkPick;
  }
  PotionType GetPotionType() override { return BOMB_NUM; }
};
