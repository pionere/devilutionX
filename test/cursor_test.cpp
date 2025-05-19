#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Cursor, SetCursor)
{
	int i = dvl::CURSOR_FIRSTITEM + dvl::ICURS_SPIKED_CLUB;
	NewCursor(i);
	EXPECT_EQ(dvl::pcursicon, i);
	EXPECT_EQ(dvl::cursW, 1 * 28);
	EXPECT_EQ(dvl::cursH, 3 * 28);
}
