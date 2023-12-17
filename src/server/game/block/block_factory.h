#pragma once

#include "block_base.h"
#include "mud.h"
#include "wall.h"
#include <memory>

class BlockFactory {
public:
  static std::shared_ptr<BlockBase> GenBlock(BlockType block_type, Pos pos) {
    switch (block_type) {
    case WALL:
      return std::make_shared<Wall>(pos);
    case MUD:
      return std::make_shared<Mud>(pos);
    }
    return nullptr;
  }
};
