#include "game/print.h"
#include "game/snapshot.h"
#include <termio.h>
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <snapshot_file>" << endl;
    return 0;
  }
  string filename = argv[1];
  Snapshot snapshot;
  string err_msg;
  // step0:read form file
  std::ifstream file(filename);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  if (content.empty()) {
    cout << "Read file failed: " << filename << endl;
    return 0;
  }

  // step1:init
  auto rc = snapshot.Init(content, err_msg);
  if (rc != 0) {
    cout << "Init failed: " << err_msg << endl;
    return 0;
  }
  while (snapshot.GetStatusMachineStatus() ==
         Snapshot::STATUS_MACHINE_STATUS_NORMAL) {
    auto ch = getch();
    switch (ch) {
    case ' ':
    case 'd':
      rc = snapshot.NextRound(err_msg);
      if (rc != 0) {
        cout << "NextRound failed: " << err_msg << endl;
        return 0;
      }
      break;
    case 'a':
      rc = snapshot.LastRound(err_msg);
      if (rc != 0) {
        cout << "LastRound failed: " << err_msg << endl;
        return 0;
      }
      break;
    case 's':
      rc = snapshot.NextStep(err_msg);
      if (rc != 0) {
        cout << "NextStep failed: " << err_msg << endl;
        return 0;
      }
      break;
    case 'w':
      rc = snapshot.LastStep(err_msg);
      if (rc != 0) {
        cout << "LastStep failed: " << err_msg << endl;
        return 0;
      }
      break;
    case 'q':
      return 0;
    }
    if (ch == 3 || ch == 33) {
      // ctrl-c
      return 0;
    }
    Print::GetInstance().PrintSnapshot(snapshot.GetNowSnapshot(),
                                       snapshot.GetNowRound());
    cout << endl;
  }
  return 0;
}
