#pragma once
#include "action.h"
#include "area.h"
#include "block/block_base.h"
#include "bomb.h"
#include "logger.h"
#include "random.h"
#include <cstdlib>
#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <unordered_map>
#include <vector>

enum GameStatus {
  UNINIT = 0,  // 未初始化
  WAIT_PLAYER, // 等待玩家
  WAIT_ACTION, // 等待动作
  FLUSHING,    // 计算结果中
  GAME_OVER,   // 游戏结束
};

class Game {
public:
  Game(const Game &) = delete;
  Game &operator=(const Game &) = delete;
  static Game &GetInstance() {
    static Game game;
    return game;
  }
  inline GameStatus game_status() { return game_status_; }
  inline const std::vector<std::vector<std::shared_ptr<Area>>> &map() {
    return map_;
  }
  inline int game_now_round() { return game_now_round_; };
  inline std::unordered_map<ID, std::shared_ptr<Player>> &player_map() {
    return player_map_;
  }
  inline std::unordered_map<ID, std::shared_ptr<Bomb>> &bomb_map() {
    return bomb_map_;
  }
  inline std::unordered_map<ID, std::shared_ptr<BlockBase>> &block_map() {
    return block_map_;
  }

  RC Init();
  RC AddPlayer(ID &player_id, const std::string &player_name = "unknown");
  RC AddAction(Action action);
  // win_players_id 为获胜者的ids
  RC FlushTime(std::vector<ID> &win_players_id);
  const std::shared_ptr<Player> GetPlayerByID(ID player_id);
  const std::shared_ptr<BlockBase> GetBlockByID(ID block_id);
  const std::shared_ptr<Bomb> GetBombByID(ID bomb_id);
  const int GetActionNumByID(ID play_id);
  RC Reset();

private:
  ID CreatePlayer(Pos pos, const std::string &player_name);
  ID CreateBomb(Pos pos, int range, ID play_id);
  ID CreateBlock(Pos pos, BlockType block_type);
  RC DealPlaceAction(Action action);
  RC DealMoveAction(Action action);
  void CleanPlayer(ID play_id);
  void CleanBomb(ID bomb_id);
  void CleanBLock(ID block_id);
  RC FlushPlayer();
  RC FlushBomb();
  RC FlushBombMove();
  RC FlushBombExplode();
  int ProbabilityCreate();
  bool IsGameOver(std::vector<ID> &win_players_id);
  bool IsCreateWall(Pos pos);
  void PrintMap();
  RC InitMap();

private:
  std::vector<std::vector<std::shared_ptr<Area>>> map_;
  std::unordered_map<ID, std::shared_ptr<Player>> player_map_;
  std::unordered_map<ID, std::shared_ptr<BlockBase>> block_map_;
  std::unordered_map<ID, std::shared_ptr<Bomb>> bomb_map_;
  std::unordered_map<ID, int> action_player_map_;

private:
  Game() {
    std::vector<spdlog::sink_ptr> sinks;

    if (kIsLogPrintStdout) {
      sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
    sinks.push_back(
        std::make_shared<spdlog::sinks::daily_file_sink_st>("seedcup", 23, 59));
    logger_ = setup_logger("seedgame", sinks);

    // set logger level
    spdlog::set_level(spdlog::level::level_enum::debug);
    logger_->debug("setup logger success");
  }

private:
  GameStatus game_status_ = UNINIT;
  int game_now_round_ = 0;
  std::shared_ptr<spdlog::logger> logger_{nullptr};
};
