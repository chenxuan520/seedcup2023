#include "game.h"
#include "block/block_factory.h"
#include "const.h"
#include "custom_map.h"
#include "potion/potion_factory.h"
#include "print.h"
#include "random.h"
#include "snapshot.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <vector>

ID Game::CreatePlayer(Pos pos, const std::string &player_name) {
  auto player = std::make_shared<Player>(pos, player_name);
  ID play_id = player->play_id();
  player_map_[play_id] = player;
  return play_id;
}

ID Game::CreateBomb(Pos pos, int range, ID play_id) {
  auto bomb = std::make_shared<Bomb>(pos, range, play_id);
  bomb_map_[bomb->bomb_id()] = bomb;
  return bomb->bomb_id();
}

ID Game::CreateBlock(Pos pos, BlockType block_type) {
  std::shared_ptr<BlockBase> block = BlockFactory::GenBlock(block_type, pos);
  block_map_[block->block_id()] = block;
  return block->block_id();
}

RC Game::Init() {
  if (game_status_ != UNINIT) {
    logger_->warn("game engine has init");
    return INVALUE_OPER;
  }
  if (kIsExistCustomMap) {
    // 自定义地图
    auto rc = InitCustomMap();
    if (rc != RC::SUCCESS) {
      return rc;
    }
    game_status_ = GameStatus::WAIT_PLAYER;
    return RC::SUCCESS;
  }

  auto rc = InitMap();
  if (rc != RC::SUCCESS) {
    return rc;
  }
  rc = InitPlayerBirth();
  if (rc != RC::SUCCESS) {
    return rc;
  }
  game_status_ = GameStatus::WAIT_PLAYER;
  return RC::SUCCESS;
}

RC Game::Init(const std::vector<std::vector<std::string>> &custom_map) {
  if (game_status_ != UNINIT) {
    logger_->warn("game engine has init");
    return INVALUE_OPER;
  }
  auto rc = InitCustomMap(custom_map);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  game_status_ = GameStatus::WAIT_PLAYER;
  return RC::SUCCESS;
}

RC Game::InitCustomMap(
    const std::vector<std::vector<std::string>> &custom_map) {

  // init
  map_.resize(kMapDefaultSize,
              std::vector<std::shared_ptr<Area>>(kMapDefaultSize, nullptr));
  player_birth_.resize(4, {0, 0});

  if (custom_map.size() != kMapDefaultSize) {
    logger_->warn("custom map size error,should be {}*{}", kMapDefaultSize,
                  kMapDefaultSize);
    return RC::INVALUS_CUSTOM_MAP;
  }
  auto &instance = CustomMap::GetInstance();
  for (int i = 0; i < kMapDefaultSize; i++) {
    if (custom_map[i].size() != kMapDefaultSize) {
      logger_->warn("custom map size error,should be {}*{} in line {},{}",
                    kMapDefaultSize, kMapDefaultSize, i, custom_map[i].size());
      return RC::INVALUS_CUSTOM_MAP;
    }

    for (int j = 0; j < kMapDefaultSize; j++) {
      Pos pos{i, j};
      map_[i][j] = std::make_shared<Area>(pos);

      auto &print = custom_map[i][j];

      if (instance.IsPotionPrint(print)) {
        auto potion_type = instance.GetPotionTypeByPrint(print);
        map_[i][j]->set_potion_type(potion_type);

      } else if (instance.IsBlockPrint(print)) {
        auto block_type = instance.GetBlockTypeByPrint(print);
        map_[i][j]->set_block_id(CreateBlock({i, j}, block_type));

      } else if (instance.IsEmptyPrint(print)) {
        continue;

      } else if (instance.IsPlayerPrint(print)) {
        auto player_birth_id = instance.GetPlayerIDByPrint(print);
        if (player_birth_id < 0 || player_birth_id > 3) {
          logger_->warn("custom map print error,unknown player id:{},error:{}",
                        player_birth_id, instance.GetLastError());
          return RC::INVALUS_CUSTOM_MAP;
        }
        player_birth_[player_birth_id] = {i, j};

      } else {
        logger_->warn("custom map print error,unknown print:{}", print);
        return RC::INVALUS_CUSTOM_MAP;
      }
    }
  }
  if (player_birth_.size() == 0) {
    logger_->warn("player birth area is empty");
    return RC::INVALUS_CUSTOM_MAP;
  }
  return RC::SUCCESS;
}

RC Game::InitCustomMap() {
  ASSERT(kMapDefaultSize % 2 != 0);
  if (game_status_ != UNINIT) {
    logger_->warn("game engine has init");
    return INVALUE_OPER;
  }
  std::vector<std::vector<std::string>> custom_map;
  auto &instance = CustomMap::GetInstance();
  auto rc = instance.GetCustomMap(custom_map);
  if (rc != 0) {
    logger_->warn("get custom map failed,error reason:{}",
                  instance.GetLastError());
    return RC::INVALUS_CUSTOM_MAP;
  }
  return InitCustomMap(custom_map);
}

RC Game::InitPlayerBirth() {
  player_birth_.resize(4, {0, 0});
  // 初始化
  player_birth_[0] = {0, 0};
  player_birth_[1] = {kMapDefaultSize - 1, kMapDefaultSize - 1};
  player_birth_[2] = {kMapDefaultSize - 1, 0};
  player_birth_[3] = {0, kMapDefaultSize - 1};
  // 插入前随机位置
  std::random_shuffle(player_birth_.begin(), player_birth_.end());
  return RC::SUCCESS;
}

RC Game::InitMap() {
  ASSERT(kMapDefaultSize % 2 != 0);
  map_.resize(kMapDefaultSize,
              std::vector<std::shared_ptr<Area>>(kMapDefaultSize, nullptr));
  std::vector<Pos> block_arr;
  for (int i = 0; i < kMapDefaultSize; i++) {
    for (int j = 0; j < kMapDefaultSize; j++) {
      Pos pos{i, j};
      map_[i][j] = std::make_shared<Area>(pos);
      if (IsCreateWall(pos)) {
        map_[i][j]->set_block_id(CreateBlock({i, j}, BlockType::WALL));
      } else if ((kMapDefaultSize - i <= 2 || i <= 1) &&
                 (kMapDefaultSize - j <= 2 || j <= 1)) {
        // 玩家出生地,不能创建
        continue;
      } else {
        // 可以创建方块区
        block_arr.push_back({i, j});
      }
    }
  }
  // 洗牌
  std::random_shuffle(block_arr.begin(), block_arr.end());
  for (int i = 0; i < block_arr.size(); i++) {

    // 生成泥土
    if (ProbabilityCreate() >= kMudDefaultRandom) {
      continue;
    }

    Pos pos = block_arr[i];
    auto id = CreateBlock(pos, BlockType::MUD);
    map_[pos.first][pos.second]->set_block_id(id);
    // 生成药水
    if (ProbabilityCreate() < kPotionDefaultProbability) {
      block_map_[id]->set_potion_type(PotionBase::GenRandomPotion());
    }
  }
  logger_->debug("init map success {}*{}", kMapDefaultSize, kMapDefaultSize);

  return RC::SUCCESS;
}

RC Game::AddPlayer(ID &player_id, const std::string &player_name) {
  if (game_status_ != WAIT_PLAYER) {
    logger_->warn("game_status is {} not waiting player", game_status_);
    return RC::INVALUE_OPER;
  }
  if (player_map_.size() > kPlayerDefaultNum) {
    logger_->warn("player_num is {}  max is {}", player_map_.size(),
                  kPlayerDefaultNum);
    return RC::PLAYER_TOO_MUCH;
  }

  ASSERT(player_map_.size() < player_birth_.size());
  auto play_pos = player_birth_[player_map_.size()];
  player_id = CreatePlayer(play_pos, player_name);
  map_[play_pos.first][play_pos.second]->players().insert(player_id);
  logger_->debug("player {} add success in {},{}", player_id, play_pos.first,
                 play_pos.second);

  if (player_map_.size() == kPlayerDefaultNum) {
    logger_->debug("player is enough");
    game_status_ = WAIT_ACTION;

    // 初始化快照
    std::string snapshot_str = "";
    Snapshot::GetFromatStringWithMap(map_, snapshot_str);
    logger_->debug("init snapshot:{}", snapshot_str);
  }
  PrintMap();
  return RC::SUCCESS;
}

RC Game::DealPlaceAction(Action action) {
  ASSERT(action.action_type_ == ActionType::PLACED);
  auto player_id = action.player_id_;
  auto player = player_map_[player_id];
  auto pos = player->pos();
  auto area = map_[pos.first][pos.second];
  // step0:合法性判断
  if (player->bomb_now_num() >= player->bomb_max_num()) {
    logger_->warn(
        "player_id {} cannot place bomb,now bomb is {},max bomb is {}",
        action.player_id_, player->bomb_now_num(), player->bomb_max_num());
    return RC::BOMB_TOO_MUCH;
  }
  if (area->bomb_id() != -1 || area->block_id() != -1) {
    logger_->warn("play_id {} cannot place bomb,exist block {} or bomb {}",
                  action.player_id_, area->block_id(), area->bomb_id());
    return RC::BOMB_NO_ALLOW;
  }

  // step1:生成炸弹,放炸弹
  auto bomb_id = CreateBomb(pos, player->bomb_range(), player->play_id());
  area->set_bomb_id(bomb_id);
  logger_->debug("player_id {} place a bomb {} in {},{} {}", player_id, bomb_id,
                 pos.first, pos.second,
                 SnapshotString(SNAPSHOT_BOMB_APPEAR, bomb_id, pos));

  // step2:增加目前投放炸弹数量
  player->set_bomb_now_num(player->bomb_now_num() + 1);

  return RC::SUCCESS;
}

RC Game::DealMoveAction(Action action) {
  ASSERT(action.action_type_ != ActionType::PLACED);
  if (action.action_type_ == SLIENT) {
    return RC::SUCCESS;
  }
  auto player_id = action.player_id_;
  auto player = player_map_[player_id];
  auto pos = player->pos();
  auto old_pos = pos;
  switch (action.action_type_) {
  case MOVE_DOWN:
    pos.first += 1;
    break;
  case MOVE_UP:
    pos.first -= 1;
    break;
  case MOVE_RIGHT:
    pos.second += 1;
    break;
  case MOVE_LEFT:
    pos.second -= 1;
    break;
  default:
    break;
  }
  if (pos.first < 0 || pos.first >= kMapDefaultSize || pos.second < 0 ||
      pos.second >= kMapDefaultSize) {
    logger_->warn("player {} move pos fail {},{} cannot move out map",
                  player->play_id(), pos.first, pos.second);
    return RC::MOVE_OUT_MAP;
  }
  auto &area = map_[pos.first][pos.second];
  auto &old_area = map_[old_pos.first][old_pos.second];

  // step first:推炸弹判定
  if (player->has_gloves() && area->bomb_id() != -1) {
    auto bomb_id = area->bomb_id();
    auto bomb = bomb_map_[bomb_id];
    if (bomb->bomb_status() != BOMB_SILENT) {
      logger_->warn("player {} try move,bomb {} is moving,cannot be move",
                    player_id, bomb_id);
      return RC::MOVE_NOT_ALLOW;
    } else {
      logger_->debug("player {} move bomb {} success in {}", player_id, bomb_id,
                     action.action_type_);
      bomb->set_bomb_status(BombStatus(action.action_type_));
      return RC::MOVE_NOT_ALLOW;
    }
  }

  // step0:判断合法化
  if (area->block_id() != -1 || area->bomb_id() != -1) {
    logger_->warn("player {} move pos fail becase has block {} or bomb {}",
                  player_id, area->block_id(), area->bomb_id());
    return RC::MOVE_NOT_ALLOW;
  }

  // step1:判断完毕,可以移动
  auto rc = player->set_pos(pos);
  // 这里理论上一定成功
  ASSERT(rc == RC::SUCCESS);

  // step2:抹除原来地方痕迹,添加新的地方痕迹
  old_area->players().erase(player_id);
  area->players().insert(player_id);

  // step3:触发player相遇效果(为了解决无敌效果)
  for (auto other_id : area->players()) {
    if (other_id == player_id) {
      continue;
    }
    auto other_player = player_map_[other_id];
    auto mark_result = player->MeetOtherPlayer(other_player);
    if (mark_result > 0) {
      logger_->debug("player {} injuries other_player {},get {} mark",
                     player_id, other_id, mark_result);
    } else if (mark_result < 0) {
      logger_->debug(
          "other_player {} injuries player {},other_player get {} mark",
          other_id, player_id, mark_result);
    }
  }

  // step4:捡道具
  if (area->potion_type() != PotionType::NO_POTION) {
    auto potion = PotionFactory::GenPotion(area->potion_type());
    auto add_mark = potion->PickUp(player);
    player->IncrMark(add_mark);
    // 消除方块上的道具
    area->set_potion_type(PotionType::NO_POTION);

    logger_->debug("player {} pick up potion {},get mark {} {}", player_id,
                   potion->GetPotionType(), add_mark,
                   SnapshotString(SNAPSHOT_POTION_DISAPPREAR,
                                  potion->GetPotionType(), pos));
  }

  logger_->debug("player {} move to {},{} success {}{}", player_id, pos.first,
                 pos.second,
                 SnapshotString(SNAPSHOT_PLAYER_DISAPPREAR, player_id, old_pos),
                 SnapshotString(SNAPSHOT_PLAYER_APPEAR, player_id, pos));
  return RC::SUCCESS;
}

RC Game::AddAction(Action action) {
  auto player = player_map_[action.player_id_];
  if (game_status_ != WAIT_ACTION) {
    return RC::INVALUE_OPER;
  }
  if (player->status() == PlayStatus::DEAD) {
    logger_->warn("fail player {} has been dead", player->play_id());
    return RC::PLAYER_DEAD;
  }
  if (player->speed() <= action_player_map_[player->play_id()]) {
    logger_->warn("fail player {} speed max {}", player->play_id(),
                  player->speed());
    return RC::ACTION_TOO_MUCH;
  }
  // 添加操作
  action_player_map_[action.player_id_]++;
  auto rc = RC::SUCCESS;
  switch (action.action_type_) {
  case PLACED:
    // 处理放置炸弹
    rc = DealPlaceAction(action);
    break;
  default:
    // 处理移动触发
    rc = DealMoveAction(action);
    break;
  }
  if (rc != RC::SUCCESS) {
    return rc;
  }
  // 清理死人
  for (auto iter : player_map_) {
    auto player = iter.second;
    if (player->status() == DEAD) {
      CleanPlayer(iter.first);
    }
  }
  PrintMap();
  return rc;
}

const std::shared_ptr<Player> Game::GetPlayerByID(ID player_id) {
  if (player_map_.find(player_id) == player_map_.end()) {
    return nullptr;
  }
  return player_map_[player_id];
}

const std::shared_ptr<BlockBase> Game::GetBlockByID(ID block_id) {
  if (block_map_.find(block_id) == block_map_.end()) {
    return nullptr;
  }
  return block_map_[block_id];
}

const std::shared_ptr<Bomb> Game::GetBombByID(ID bomb_id) {
  if (bomb_map_.find(bomb_id) == bomb_map_.end()) {
    return nullptr;
  }
  return bomb_map_[bomb_id];
}
const int Game::GetActionNumByID(ID play_id) {
  if (action_player_map_.find(play_id) == action_player_map_.end()) {
    return -1;
  }
  return action_player_map_[play_id];
}

RC Game::FlushTime(std::vector<ID> &win_players_id) {
  if (game_status_ != WAIT_ACTION) {
    logger_->warn("game status is {} not wait action", game_status_);
    return RC::INVALUE_OPER;
  }
  game_status_ = FLUSHING;
  game_now_round_++;
  auto rc = FlushBomb();
  ASSERT(rc == RC::SUCCESS);
  rc = FlushPlayer();
  ASSERT(rc == RC::SUCCESS);
  auto result = IsGameOver(win_players_id);
  if (result) {
    game_status_ = GAME_OVER;
  } else {
    game_status_ = WAIT_ACTION;
  }
  logger_->debug("flush time success {}",
                 SnapshotString(SNAPSHOT_FLUSH, 0, {0, 0}));
  PrintMap();
  return RC::SUCCESS;
}

RC Game::FlushPlayer() {
  for (auto iter : player_map_) {
    auto player = iter.second;
    if (player->status() == DEAD) {
      continue;
    }
    player->FlushTime();
    // 清除action记录
    action_player_map_[iter.first] = 0;
  }
  return RC::SUCCESS;
}

RC Game::FlushBombMove() {
  // 计算炸弹移动
  for (auto iter : bomb_map_) {
    auto bomb = iter.second;
    auto bomb_pos = bomb->pos();
    auto old_pos = bomb_pos;
    if (bomb->bomb_status() == BOMB_SILENT) {
      continue;
    }

    switch (bomb->bomb_status()) {
    case BOMB_MOVE_UP:
      bomb_pos.first--;
      break;
    case BOMB_MOVE_DOWN:
      bomb_pos.first++;
      break;
    case BOMB_MOVE_RIGHT:
      bomb_pos.second++;
      break;
    case BOMB_MOVE_LEFT:
      bomb_pos.second--;
      break;
    case BOMB_SILENT:
      logger_->warn("bomb {} is silent cannot be move");
      break;
    }

    // step1:判断位置合法性
    if (bomb_pos.first < 0 || bomb_pos.first >= kMapDefaultSize ||
        bomb_pos.second < 0 || bomb_pos.second >= kMapDefaultSize) {
      bomb->set_bomb_status(BOMB_SILENT);
      logger_->debug("bomb {} come to edge stop", bomb->bomb_id());
      continue;
    }

    // step2:判断是否有障碍物
    auto area = map_[bomb_pos.first][bomb_pos.second];
    if (area->bomb_id() != -1 || area->block_id() != -1 ||
        area->potion_type() != NO_POTION || !area->players().empty()) {
      bomb->set_bomb_status(BOMB_SILENT);
      logger_->debug("bomb {} meet bomb or block or player stop",
                     bomb->bomb_id());
      continue;
    }

    // step3:移动炸弹
    map_[old_pos.first][old_pos.second]->set_bomb_id(-1);
    map_[bomb_pos.first][bomb_pos.second]->set_bomb_id(bomb->bomb_id());
    bomb->set_pos(bomb_pos);
    logger_->debug("bomb {} move to {},{} {}{}", bomb->bomb_id(),
                   bomb_pos.first, bomb_pos.second,
                   SnapshotString(SNAPSHOT_BOMB_DISAPPREAR, 0, old_pos),
                   SnapshotString(SNAPSHOT_BOMB_APPEAR, 0, bomb_pos));
  }
  return RC::SUCCESS;
}

RC Game::FlushBombExplode() {
  // 计算炸弹爆炸
  std::queue<ID> bomb_que;
  for (auto iter : bomb_map_) {
    auto bomb = iter.second;
    if (bomb->FlushTime()) {
      bomb_que.push(bomb->bomb_id());
    }
  }

  while (!bomb_que.empty()) {
    auto bomb_id = bomb_que.front();
    bomb_que.pop();
    if (GetBombByID(bomb_id) == nullptr) {
      continue;
    }
    auto bomb = bomb_map_[bomb_id];
    auto pos = bomb->pos();
    auto range = bomb->bomb_range();
    logger_->debug("bomb explosion in {},{} range {} {}", pos.first, pos.second,
                   range,
                   SnapshotString(SNAPSHOT_BOMB_DISAPPREAR, bomb_id, pos));

    // step1:清除炸弹
    CleanBomb(bomb_id);
    auto owner = player_map_[bomb->player_id()];
    owner->set_bomb_now_num(owner->bomb_now_num() - 1);

    // step2:设置爆炸函数
    auto bomb_call_back = [&](int x, int y) -> bool {
      auto area = map_[x][y];
      area->set_last_bomb_round(game_now_round_);
      // 炸人伤害
      for (auto play_id : area->players()) {
        auto player = player_map_[play_id];
        auto injured_mark = player->Injuries();
        // 这里设置成炸自己不能加分数
        if (play_id != owner->play_id()) {
          owner->IncrMark(injured_mark);
          logger_->debug("{} player's bomb injuries other_player {},in "
                         "{},{},get mark {} owner'mark {}",
                         owner->play_id(), play_id, x, y, injured_mark,
                         owner->mark());
        } else {
          logger_->debug("{} player bomb self", owner->play_id());
        }
      }
      // 炸炸弹
      if (area->bomb_id() != -1) {
        bomb_que.push(area->bomb_id());
        return true;
      }
      // 炸道具
      if (area->potion_type() != NO_POTION) {
        logger_->debug("potion in {},{} disappeared {}", x, y,
                       SnapshotString(SNAPSHOT_POTION_DISAPPREAR,
                                      area->potion_type(), {x, y}));
        area->set_potion_type(NO_POTION);
      }
      // 炸方块
      if (area->block_id() != -1) {
        auto block = block_map_[area->block_id()];
        if (block->IsBombAble()) {
          auto result = block->BombInjuries();
          owner->IncrMark(result.first);
          area->set_potion_type(result.second);
          // 清除方块
          CleanBLock(area->block_id());
          logger_->debug(
              "block in {},{} disappeared {}{}", x, y,
              SnapshotString(SNAPSHOT_BLOCK_DISAPPREAR, 0, {x, y}),
              SnapshotString(SNAPSHOT_POTION_APPEAR, result.second, {x, y}));
        }
        return true;
      }
      return false;
    };

    // 选择范围爆炸
    bomb_call_back(pos.first, pos.second);
    for (int j = 0; j < 4; j++) {
      for (int i = 1; i <= range; i++) {
        int x = pos.first, y = pos.second;
        switch (j) {
        case 0:
          x += i;
          break;
        case 1:
          x -= i;
          break;
        case 2:
          y += i;
          break;
        case 3:
          y -= i;
          break;
        }
        if (x < 0 || x >= kMapDefaultSize || y < 0 || y >= kMapDefaultSize) {
          continue;
        }
        if (bomb_call_back(x, y)) {
          // 炸到墙壁或者炸弹停止
          break;
        }
      }
    }
  }
  return RC::SUCCESS;
}

RC Game::FlushBomb() {
  auto rc = FlushBombMove();
  if (rc != RC::SUCCESS) {
    return rc;
  }
  return FlushBombExplode();
}

bool Game::IsGameOver(std::vector<ID> &win_players_id) {
  win_players_id.clear();
  int alive_num = 0, alive_player_id = -1, mark_max = INT32_MIN;
  for (auto iter : player_map_) {
    auto player = iter.second;
    if (player->mark() > mark_max) {
      mark_max = player->mark();
      win_players_id.clear();
      win_players_id.push_back(player->play_id());
    } else if (player->mark() == mark_max) {
      win_players_id.push_back(player->play_id());
    }

    if (player->status() == PlayStatus::ALIVE) {
      alive_num++;
      alive_player_id = player->play_id();
    } else {
      logger_->debug("player {} is dead {}", player->play_id(),
                     SnapshotString(SNAPSHOT_PLAYER_DISAPPREAR,
                                    player->play_id(), player->pos()));
      // 清理死人
      CleanPlayer(player->play_id());
    }
  }
  if (alive_num == 1) {
    logger_->debug("game over only winner is {}", alive_player_id);
    win_players_id.clear();
    win_players_id.push_back(alive_player_id);
    return true;
  }
  if (game_now_round_ > kGameDefaultMaxRound || alive_num == 0) {
    logger_->debug("game over mark winner, mark is {}", mark_max);
    return true;
  }
  logger_->debug("game continue now alive num {},round {},max round {}",
                 alive_num, game_now_round_, kGameDefaultMaxRound);
  return false;
}

RC Game::Reset() {
  player_map_.clear();
  block_map_.clear();
  bomb_map_.clear();
  game_status_ = UNINIT;
  game_now_round_ = 0;
  map_ = std::vector<std::vector<std::shared_ptr<Area>>>();
  return RC::SUCCESS;
}

void Game::CleanPlayer(ID play_id) {
  if (player_map_.find(play_id) == player_map_.end()) {
    return;
  }
  auto player = player_map_[play_id];
  if (player->status() != DEAD) {
    return;
  }
  if (player->pos().first == -1) {
    return;
  }
  auto pos = player->pos();
  auto area = map_[pos.first][pos.second];
  area->players().erase(play_id);
}

void Game::CleanBomb(ID bomb_id) {
  if (bomb_map_.find(bomb_id) == bomb_map_.end()) {
    return;
  }
  auto pos = bomb_map_[bomb_id]->pos();
  ASSERT(pos.first != -1);
  bomb_map_.erase(bomb_id);
  map_[pos.first][pos.second]->set_bomb_id(-1);
}

void Game::CleanBLock(ID block_id) {
  if (block_map_.find(block_id) == block_map_.end()) {
    return;
  }
  auto pos = block_map_[block_id]->pos();
  ASSERT(pos.first != -1);
  block_map_.erase(block_id);
  map_[pos.first][pos.second]->set_block_id(-1);
}

bool Game::IsCreateWall(Pos pos) {
  auto i = pos.first, j = pos.second;
  if (i % 2 != 0 && j % 2 != 0) {
    // 固定创建墙壁
    return true;
  } else if ((i + j) % 2 != 0 &&
             (i >= 1 && i < kMapDefaultSize - 1 && j >= 1 &&
              j < kMapDefaultSize - 1) &&
             ProbabilityCreate() < kWallDefaultRandom) {
    // 随机创建墙壁
    return true;
  } else if (((j == kMapDefaultSize / 2 &&
               (i == 0 || i == kMapDefaultSize - 1)) ||
              (i == kMapDefaultSize / 2 &&
               (j == 0 || j == kMapDefaultSize - 1))) &&
             ProbabilityCreate() < kWallDefaultRandom) {
    // 创建4个角落墙壁
    return true;
  }
  return false;
}

int Game::ProbabilityCreate() {
  static Random probability_creater(0, 100, kSeedRandom);
  return probability_creater.CreateRandom();
}

void Game::PrintMap() {
  if (kIsGamePrintMap) {
    Print::GetInstance().PrintMap(*this);
  }
}

std::string Game::SnapshotString(SnapshotEventType event_type, int affect_id,
                                 const std::pair<int, int> &pos) {
  if (kIsSnapshot) {
    std::string result;
    Snapshot::GetFormatString(event_type, affect_id, pos, result);
    return result;
  }
  return "";
}
