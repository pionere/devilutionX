#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Diablo, diablo_pause_game_unpause)
{
	diablo_pause_game(true);
	EXPECT_EQ(gnGamePaused != 0u, true);
	diablo_pause_game(true);
	EXPECT_EQ(gnGamePaused != 0u, true);
	diablo_pause_game(false);
	EXPECT_EQ(gnGamePaused, 0u);
	diablo_pause_game(false);
	EXPECT_EQ(gnGamePaused, 0u);
}
