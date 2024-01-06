#pragma once

#include "../server/game/game.h"
#include "test.h"
#include <vector>

// 13*13 vector
std::vector<std::vector<std::string>> map = {
    {"0", "1", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
    {"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_"},
};
auto &game = Game::GetInstance();

TEST(Game, Init) {
  auto rc = game.Reset();
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), UNINIT);

  rc = game.Init(map);
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);

  rc = game.Reset();
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), UNINIT);

  rc = game.Init();
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);
}

TEST(Game, AddPlayer) {
  auto rc = game.Reset();
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), UNINIT);

  rc = game.Init(map);
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);

  ID player_id_0 = 0;
  rc = game.AddPlayer(player_id_0, "player0");
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);

  ID player_id_1 = 0;
  rc = game.AddPlayer(player_id_1, "player1");
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_ACTION);

  ID temp_id = 0;
  rc = game.AddPlayer(temp_id, "temp_id");
  MUST_EQUAL(rc, INVALUE_OPER);
  MUST_EQUAL(game.game_status(), WAIT_ACTION);

  auto player0 = game.GetPlayerByID(player_id_0);
  auto palyer1 = game.GetPlayerByID(player_id_1);
  MUST_EQUAL(player0->player_name(), "player0");
  MUST_EQUAL(palyer1->player_name(), "player1");
  MUST_EQUAL(player0->pos().first, 0);
  MUST_EQUAL(player0->pos().second, 0);
  MUST_EQUAL(palyer1->pos().first, 0);
  MUST_EQUAL(palyer1->pos().second, 1);
}

TEST(Game, AddAction) {
  auto rc = game.Reset();
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), UNINIT);

  rc = game.Init(map);
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);

  ID player_id_0 = 0;
  rc = game.AddPlayer(player_id_0, "player0");
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_PLAYER);

  ID player_id_1 = 0;
  rc = game.AddPlayer(player_id_1, "player1");
  MUST_EQUAL(rc, SUCCESS);
  MUST_EQUAL(game.game_status(), WAIT_ACTION);

  rc = game.AddAction(Action(player_id_0, ActionType::MOVE_UP));
  MUST_TRUE(rc != SUCCESS, "player0 can't move up");
  rc = game.AddAction(Action(player_id_0, ActionType::MOVE_DOWN));
  MUST_EQUAL(rc, SUCCESS);
  auto player0 = game.GetPlayerByID(player_id_0);
  MUST_EQUAL(player0->pos().first, 1);
  MUST_EQUAL(player0->pos().second, 0);
}
