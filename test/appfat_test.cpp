#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Appfat, app_fatal)
{
	EXPECT_EXIT(app_fatal("test"), ::testing::ExitedWithCode(1), "test");
}
