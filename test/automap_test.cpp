#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Automap, InitAutomap)
{
	dvl::InitAutomapOnce();
	EXPECT_EQ(dvl::gbAutomapflag, AMM_NONE);
	EXPECT_EQ(dvl::AutoMapScale, 64u); // MAP_SCALE_NORMAL
	EXPECT_EQ(dvl::IsAutomapActive(), false);
	
	// EXPECT_EQ(dvl::AmLine64, 32);
	// EXPECT_EQ(dvl::AmLine32, 16);
	// EXPECT_EQ(dvl::AmLine16, 8);
}

TEST(Automap, StartAutomap)
{
	dvl::InitAutomapOnce();

	dvl::ToggleAutomap();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
	EXPECT_EQ(dvl::gbAutomapflag, AMM_MINI);
	EXPECT_EQ(dvl::IsAutomapActive(), true);
	dvl::ToggleAutomap();
	EXPECT_EQ(dvl::gbAutomapflag, AMM_NORMAL);
	EXPECT_EQ(dvl::IsAutomapActive(), true);
	dvl::ToggleAutomap();
	EXPECT_EQ(dvl::gbAutomapflag, AMM_NONE);
	EXPECT_EQ(dvl::IsAutomapActive(), false);
}

TEST(Automap, AutomapUp)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapUp();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
}

TEST(Automap, AutomapDown)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapDown();
	EXPECT_EQ(dvl::AutoMapXOfs, 2);
	EXPECT_EQ(dvl::AutoMapYOfs, 2);
}

TEST(Automap, AutomapLeft)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapLeft();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 2);
}

TEST(Automap, AutomapRight)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapRight();
	EXPECT_EQ(dvl::AutoMapXOfs, 2);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
}

TEST(Automap, AutomapZoomIn)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutomapZoomIn();
	EXPECT_EQ(dvl::AutoMapScale, 64u + 16);
	EXPECT_EQ(dvl::MiniMapScale, 64u + 16);
}

TEST(Automap, AutomapZoomIn_Max)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutomapZoomIn();
	dvl::AutomapZoomIn();
	dvl::AutomapZoomIn();
	dvl::AutomapZoomIn();
	EXPECT_EQ(dvl::AutoMapScale, 64u + 16 * 4);
	EXPECT_EQ(dvl::MiniMapScale, 64u + 16 * 4);
	dvl::AutomapZoomIn();
	EXPECT_EQ(dvl::AutoMapScale, 64u + 16 * 4);
	EXPECT_EQ(dvl::MiniMapScale, 64u + 16 * 4);
}

TEST(Automap, AutomapZoomOut)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutomapZoomOut();
	EXPECT_EQ(dvl::AutoMapScale, 64u - 16);
	EXPECT_EQ(dvl::MiniMapScale, 64u - 16);
}

TEST(Automap, AutomapZoomOut_Min)
{
	dvl::InitAutomapOnce();
	dvl::ToggleAutomap();

	dvl::AutomapZoomOut();
	dvl::AutomapZoomOut();
	dvl::AutomapZoomOut();
	EXPECT_EQ(dvl::AutoMapScale, 64u - 16 * 3);
	EXPECT_EQ(dvl::MiniMapScale, 64u - 16 * 3);
	dvl::AutomapZoomOut();
	EXPECT_EQ(dvl::AutoMapScale, 64u - 16 * 3);
	EXPECT_EQ(dvl::MiniMapScale, 64u - 16 * 3);
}
