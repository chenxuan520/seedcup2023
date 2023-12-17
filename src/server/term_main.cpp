#include "config.h"
#include "game/game.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <termio.h>
#include <unordered_map>
#include <vector>
// #include "net/server.h"
using namespace std;
int getch(void) {
  struct termios tm, tm_old;
  int fd = 0, ch;

  if (tcgetattr(fd, &tm) < 0) { //保存现在的终端设置
    return -1;
  }

  tm_old = tm;
  cfmakeraw(
      &tm); //更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
  if (tcsetattr(fd, TCSANOW, &tm) < 0) { //设置上更改之后的设置
    return -1;
  }

  ch = getchar();
  if (tcsetattr(fd, TCSANOW, &tm_old) < 0) { //更改设置为最初的样子
    return -1;
  }

  return ch;
}
int main() {
  auto &game = Game::GetInstance();
  while (1) {
    Game::GetInstance().Init();
    vector<ID> player_ids(kPlayerDefaultNum, -1);
    for (int i = 0; i < kPlayerDefaultNum; i++) {
      Game::GetInstance().AddPlayer(player_ids[i]);
    }
    vector<ID> winers;
    while (game.game_status() != GAME_OVER) {
      for (int i = 0; i < kPlayerDefaultNum; i++) {
        fflush(stdin);
        auto ch = getch();
        switch (ch) {
        case 'w':
          game.AddAction(Action(player_ids[0], MOVE_UP));
          break;
        case 's':
          game.AddAction(Action(player_ids[0], MOVE_DOWN));
          break;
        case 'a':
          game.AddAction(Action(player_ids[0], MOVE_LEFT));
          break;
        case 'd':
          game.AddAction(Action(player_ids[0], MOVE_RIGHT));
          break;
        case ' ':
          game.AddAction(Action(player_ids[0], PLACED));
          break;
        case 'q':
          game.AddAction(Action(player_ids[0], SLIENT));
          break;
        case 'i':
          game.AddAction(Action(player_ids[1], MOVE_UP));
          break;
        case 'k':
          game.AddAction(Action(player_ids[1], MOVE_DOWN));
          break;
        case 'j':
          game.AddAction(Action(player_ids[1], MOVE_LEFT));
          break;
        case 'l':
          game.AddAction(Action(player_ids[1], MOVE_RIGHT));
          break;
        case 'o':
          game.AddAction(Action(player_ids[1], PLACED));
          break;
        case 'u':
          game.AddAction(Action(player_ids[2], SLIENT));
          break;
        case 'g':
          game.AddAction(Action(player_ids[2], MOVE_UP));
          break;
        case 'b':
          game.AddAction(Action(player_ids[2], MOVE_DOWN));
          break;
        case 'v':
          game.AddAction(Action(player_ids[2], MOVE_LEFT));
          break;
        case 'n':
          game.AddAction(Action(player_ids[2], MOVE_RIGHT));
          break;
        case 'h':
          game.AddAction(Action(player_ids[2], PLACED));
          break;
        case 'f':
          game.AddAction(Action(player_ids[2], SLIENT));
          break;
        case 'x':
          game.FlushTime(winers);
          break;
        }
        if (ch == 3 || ch == 33) {
          // ctrl-c
          return 0;
        }
      }
      game.FlushTime(winers);
    }
    game.Reset();
  }
  return 0;
}
