#pragma once
#include "const.h"
#include "potion/potion_base.h"
#include <unordered_set>

class Area final {
private:
  Pos pos_{-1, -1};
  std::unordered_set<ID> players_;
  ID bomb_id_ = -1;
  ID block_id_ = -1;
  PotionType potion_type_ = NO_POTION;
  int last_bomb_round_ = -1;

public:
  Area(Pos pos) : pos_(pos){};
  inline RC set_bomb_id(ID bomb_id) {
    bomb_id_ = bomb_id;
    return RC::SUCCESS;
  }
  inline RC set_block_id(ID block_id) {
    block_id_ = block_id;
    return RC::SUCCESS;
  }
  inline RC set_potion_type(PotionType potion_type) {
    potion_type_ = potion_type;
    return RC::SUCCESS;
  }
  inline RC set_last_bomb_round(int round) {
    last_bomb_round_ = round;
    return RC::SUCCESS;
  }
  Pos pos() { return pos_; };
  std::unordered_set<int> &players() { return players_; }
  inline ID bomb_id() const { return bomb_id_; }
  inline ID block_id() const { return block_id_; }
  inline PotionType potion_type() const { return potion_type_; }
  inline int last_bomb_round() const { return last_bomb_round_; }
};
