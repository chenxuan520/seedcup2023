#pragma once
#include "block_base.h"
#include <memory>

class Mud : public BlockBase {
public:
  Mud(Pos pos) : BlockBase(pos) {}
  bool IsBombAble() const override { return true; }
  BlockType block_type() const override { return MUD; }
  std::pair<int, PotionType> BombInjuries() override {
    return {kMarkBombMud, potion_};
  }
};
