#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Diablo, diablo_pause_game_unpause)
{
	gbGamePaused = true;
	diablo_pause_game();
	EXPECT_EQ(gbGamePaused, false);
}
