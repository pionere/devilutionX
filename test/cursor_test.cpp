#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Cursor, SetCursor)
{
	int i = ICURS_SPIKED_CLUB + CURSOR_FIRSTITEM;
	NewCursor(i);
	EXPECT_EQ(pcurs, i);
	EXPECT_EQ(cursW, 1 * 28);
	EXPECT_EQ(cursH, 3 * 28);
}
