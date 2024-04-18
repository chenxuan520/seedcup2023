#include "game_test.h"
#include "snapshot_test.h"
#include "test.h"

ARGC_FUNC {
  if (argc == 2) {
    REGEX_FILT_TEST(argv[1]);
  }
}

TEST(InitTry, Num) { EXPECT_EQ(1, 1); }
