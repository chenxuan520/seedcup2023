#pragma once
#include "block_base.h"

class Wall : public BlockBase {
public:
  Wall(Pos pos) : BlockBase(pos) {}
  bool IsBombAble() const override { return false; }
  BlockType block_type() const override { return WALL; }
};
