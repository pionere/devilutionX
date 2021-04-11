#include <gtest/gtest.h>
#include "all.h"

namespace dvl {
extern BYTE predungeon[DMAXX][DMAXY];
extern void DRLG_L2InitDungeon();
}

TEST(Drlg_l2, DRLG_L2InitDungeon)
{
	dvl::DRLG_L2InitDungeon();
	EXPECT_EQ(dvl::predungeon[0][0], 32);
	EXPECT_EQ(dvl::dflags[0][0], 0);
}
