#pragma once

#include "../server/game/print.h"
#include "../server/game/snapshot.h"
#include "test.h"

TEST(Snapshot, GetFormatString) {
  Snapshot snapshot;
  std::string result;
  std::vector<int> affect_arr_id;
  snapshot.GetFormatString(SnapshotEventType::SNAPSHOT_PLAYER_APPEAR, 0, {0, 0},
                           result);
  MUST_EQUAL(result, "<0 0 [0 0]>");
}

TEST(Snapshot, GetFromatStringWithMap) {
  Snapshot snapshot;
  std::string result;
  std::vector<std::vector<std::shared_ptr<Area>>> map;
  std::vector<std::shared_ptr<Area>> row;
  std::shared_ptr<Area> area;
  Pos pos = {0, 0};
  area = std::make_shared<Area>(pos);
  area->set_block_id(0);
  row.push_back(area);
  area = std::make_shared<Area>(pos);
  area->players().insert(0);
  row.push_back(area);
  map.push_back(row);
  row.clear();

  snapshot.GetFromatStringWithMap(map, result);
  MUST_EQUAL(result, "<{\"map\":[\"50\"]}>");
}

TEST(Snapshot, Init) {
  Snapshot snapshot;
  std::string err_msg;
  std::string log_str = "<{\"map\":[\"04\",\"65\"]}> <1 0 [0 0]> <0 0 [0 1]>";
  auto rc = snapshot.Init(log_str, err_msg);
  MUST_EQUAL(err_msg, "");
  MUST_EQUAL(rc, 0);
  auto map = snapshot.GetNowSnapshot();
  MUST_EQUAL(map.size(), 2);
  Print::GetInstance().PrintSnapshot(snapshot.GetNowSnapshot(),
                                     snapshot.GetNowRound());
}

TEST(Snapshot, Comprehensive) {
  Snapshot snapshot;
  std::string result;
  std::vector<std::vector<std::shared_ptr<Area>>> map(
      4, std::vector<std::shared_ptr<Area>>(4, nullptr));
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      map[i][j] = std::make_shared<Area>(Pos{i, j});
    }
  }
  ID player_id = 1;
  map[0][0]->set_block_id(1);
  map[0][1]->players().insert(player_id);

  std::string temp = "";
  snapshot.GetFromatStringWithMap(map, result);

  snapshot.GetFormatString(SNAPSHOT_BOMB_APPEAR, 0, {0, 1}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_PLAYER_DISAPPREAR, player_id, {0, 1}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_PLAYER_APPEAR, player_id, {0, 2}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_FLUSH, 0, {0, 0}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_PLAYER_DISAPPREAR, player_id, {0, 2}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_PLAYER_APPEAR, player_id, {0, 3}, temp);
  result += temp;

  snapshot.GetFormatString(SNAPSHOT_FLUSH, 0, {0, 0}, temp);
  result += temp;
  snapshot.GetFormatString(SNAPSHOT_FLUSH, 0, {0, 0}, temp);
  result += temp;

  std::string err_msg;
  auto rc = snapshot.Init(result, err_msg);
  MUST_EQUAL(err_msg, "");
  MUST_EQUAL(rc, 0);

  Print::GetInstance().PrintSnapshot(snapshot.GetNowSnapshot(),
                                     snapshot.GetNowRound());

  rc = snapshot.NextRound(err_msg);
  MUST_EQUAL(err_msg, "");
  MUST_EQUAL(rc, 0);

  Print::GetInstance().PrintSnapshot(snapshot.GetNowSnapshot(),
                                     snapshot.GetNowRound());

  rc=snapshot.NextRound(err_msg);
  MUST_EQUAL(err_msg, "");
  MUST_EQUAL(rc, 0);

  rc = snapshot.LastRound(err_msg);
  MUST_EQUAL(err_msg, "");
  MUST_EQUAL(rc, 0);

  Print::GetInstance().PrintSnapshot(snapshot.GetNowSnapshot(),
                                     snapshot.GetNowRound());
}
