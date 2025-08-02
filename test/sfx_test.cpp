#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

/*TEST(Sfx, calc_snd_position_center)
{
	myplr._px = 50;
	myplr._py = 50;
	int plVolume = 0;
	int plPan = 0;
	EXPECT_EQ(calc_snd_position(50, 50, &plVolume, &plPan), true);
	EXPECT_EQ(plVolume, 0);
	EXPECT_EQ(plPan, 0);
}

TEST(Sfx, calc_snd_position_near)
{
	myplr._px = 50;
	myplr._py = 50;
	int plVolume = 0;
	int plPan = 0;
	EXPECT_EQ(calc_snd_position(55, 50, &plVolume, &plPan), true);
	EXPECT_EQ(plVolume, -320);
	EXPECT_EQ(plPan, 1280);
}

TEST(Sfx, calc_snd_position_out_of_range)
{
	myplr._px = 12;
	myplr._py = 12;
	int plVolume = 0;
	int plPan = 0;
	EXPECT_EQ(calc_snd_position(112, 112, &plVolume, &plPan), false);
	ASSERT_GE(plVolume, 6400);
	EXPECT_EQ(plPan, 0);
}

TEST(Sfx, calc_snd_position_extream_right)
{
	myplr._px = 50;
	myplr._py = 50;
	int plVolume = 0;
	int plPan = 0;
	EXPECT_EQ(calc_snd_position(76, 50, &plVolume, &plPan), false);
	EXPECT_EQ(plVolume, 0);
	EXPECT_GT(plPan, 6400);
}

TEST(Sfx, calc_snd_position_extream_left)
{
	myplr._px = 50;
	myplr._py = 50;
	int plVolume = 0;
	int plPan = 0;
	EXPECT_EQ(calc_snd_position(24, 50, &plVolume, &plPan), false);
	EXPECT_EQ(plVolume, 0);
	EXPECT_LT(plPan, -6400);
}*/
