#include <gtest/gtest.h>
#include "all.h"

namespace dvl {
extern unsigned AmLine64;
extern unsigned AmLine32;
extern unsigned AmLine16;
extern unsigned AmLine8;
extern unsigned AmLine4;
}

TEST(Automap, InitAutomap) {
	dvl::InitAutomapOnce();
	EXPECT_EQ(dvl::automapflag, false);
	EXPECT_EQ(dvl::AutoMapScale, 64);
	EXPECT_EQ(dvl::AmLine64, 32);
	EXPECT_EQ(dvl::AmLine32, 16);
	EXPECT_EQ(dvl::AmLine16, 8);
	EXPECT_EQ(dvl::AmLine8, 4);
	EXPECT_EQ(dvl::AmLine4, 2);
}

TEST(Automap, StartAutomap) {
	dvl::StartAutomap();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
	EXPECT_EQ(dvl::automapflag, true);
}

TEST(Automap, AutomapUp) {
	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapUp();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
}

TEST(Automap, AutomapDown) {
	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapDown();
	EXPECT_EQ(dvl::AutoMapXOfs, 2);
	EXPECT_EQ(dvl::AutoMapYOfs, 2);
}

TEST(Automap, AutomapLeft) {
	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapLeft();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 2);
}

TEST(Automap, AutomapRight) {
	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapRight();
	EXPECT_EQ(dvl::AutoMapXOfs, 2);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
}

TEST(Automap, AutomapZoomIn) {
	dvl::AutoMapScale = 64;
	dvl::AutomapZoomIn();
	EXPECT_EQ(dvl::AutoMapScale, 76);
	EXPECT_EQ(dvl::AmLine64, 38);
	EXPECT_EQ(dvl::AmLine32, 19);
	EXPECT_EQ(dvl::AmLine16, 9);
	EXPECT_EQ(dvl::AmLine8, 4);
	EXPECT_EQ(dvl::AmLine4, 2);
}

TEST(Automap, AutomapZoomIn_Max) {
	dvl::AutoMapScale = 244;
	dvl::AutomapZoomIn();
	dvl::AutomapZoomIn();
	EXPECT_EQ(dvl::AutoMapScale, 256);
	EXPECT_EQ(dvl::AmLine64, 128);
	EXPECT_EQ(dvl::AmLine32, 64);
	EXPECT_EQ(dvl::AmLine16, 32);
	EXPECT_EQ(dvl::AmLine8, 16);
	EXPECT_EQ(dvl::AmLine4, 8);
}

TEST(Automap, AutomapZoomOut) {
	dvl::AutoMapScale = 256;
	dvl::AutomapZoomOut();
	EXPECT_EQ(dvl::AutoMapScale, 244);
	EXPECT_EQ(dvl::AmLine64, 122);
	EXPECT_EQ(dvl::AmLine32, 61);
	EXPECT_EQ(dvl::AmLine16, 30);
	EXPECT_EQ(dvl::AmLine8, 15);
	EXPECT_EQ(dvl::AmLine4, 7);
}

TEST(Automap, AutomapZoomOut_Min) {
	dvl::AutoMapScale = 76;
	dvl::AutomapZoomOut();
	dvl::AutomapZoomOut();
	EXPECT_EQ(dvl::AutoMapScale, 64);
	EXPECT_EQ(dvl::AmLine64, 32);
	EXPECT_EQ(dvl::AmLine32, 16);
	EXPECT_EQ(dvl::AmLine16, 8);
	EXPECT_EQ(dvl::AmLine8, 4);
	EXPECT_EQ(dvl::AmLine4, 2);
}

TEST(Automap, AutomapZoomReset) {
	dvl::AutoMapScale = 64;
	dvl::AutoMapXOfs = 1;
	dvl::AutoMapYOfs = 1;
	dvl::AutomapZoomReset();
	EXPECT_EQ(dvl::AutoMapXOfs, 0);
	EXPECT_EQ(dvl::AutoMapYOfs, 0);
	EXPECT_EQ(dvl::AutoMapScale, 64);
	EXPECT_EQ(dvl::AmLine64, 32);
	EXPECT_EQ(dvl::AmLine32, 16);
	EXPECT_EQ(dvl::AmLine16, 8);
	EXPECT_EQ(dvl::AmLine8, 4);
	EXPECT_EQ(dvl::AmLine4, 2);
}
