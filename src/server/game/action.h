#pragma once
#include "const.h"

enum ActionType {
  SLIENT = 0, // 静止不动
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  PLACED, // 放置炸弹或者道具
};

struct Action {
  ID player_id_ = -1;
  ActionType action_type_ = SLIENT;
  Action(ID player_id, ActionType type)
      : player_id_(player_id), action_type_(type) {}
  Action() {}
};
