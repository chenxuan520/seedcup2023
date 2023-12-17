#pragma once
#include "../const.h"
#include "../potion/potion_base.h"
#include <memory>
#include <unordered_map>

enum BlockType {
  MUD = 0,
  WALL,
};

class BlockBase {
protected:
  Pos pos_{-1, -1};
  ID block_id_ = -1;
  PotionType potion_ = NO_POTION;

public:
  BlockBase(Pos pos) : pos_(pos) {
    static ID block_id = 0;
    block_id_ = block_id++;
    block_id %= kIncrMaxID;
  }
  RC set_potion_type(PotionType potion) {
    ASSERT(IsBombAble());
    ASSERT(potion_ == NO_POTION);
    potion_ = potion;
    return RC::SUCCESS;
  }
  inline Pos pos() const { return pos_; }
  virtual BlockType block_type() const = 0;
  virtual ~BlockBase() {}
  virtual bool IsBombAble() const { return false; }
  virtual std::pair<int, PotionType> BombInjuries() {
    ASSERT(!IsBombAble());
    return {0, NO_POTION};
  }
  inline ID block_id() const { return block_id_; }
};
