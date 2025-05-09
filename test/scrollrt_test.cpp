#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

// CalcViewportGeometry

TEST(Scrool_rt, calc_tiles_in_view_original)
{
	screenWidth = 640 * ASSET_MPL;
	screenHeight = 480 * ASSET_MPL;
	gbZoomInFlag = false;
	CalcViewportGeometry();
	EXPECT_EQ(dvl::gsTileVp._vColumns, 10u);
	EXPECT_EQ(dvl::gsTileVp._vRows, 30u);
}

TEST(Scrool_rt, calc_tiles_in_view_original_zoom)
{
	screenWidth = 640 * ASSET_MPL;
	screenHeight = 480 * ASSET_MPL;
	gbZoomInFlag = true;
	CalcViewportGeometry();
	EXPECT_EQ(dvl::gsTileVp._vColumns, 5u);
	EXPECT_EQ(dvl::gsTileVp._vRows, 15u);
}

TEST(Scrool_rt, calc_tiles_in_view_960_480)
{
	screenWidth = 960 * ASSET_MPL;
	screenHeight = 480 * ASSET_MPL;
	gbZoomInFlag = false;
	CalcViewportGeometry();
	EXPECT_EQ(dvl::gsTileVp._vColumns, 15u);
	EXPECT_EQ(dvl::gsTileVp._vRows, 30u);
}

TEST(Scrool_rt, calc_tiles_in_view_960_480_zoom)
{
	screenWidth = 960 * ASSET_MPL;
	screenHeight = 480 * ASSET_MPL;
	gbZoomInFlag = true;
	CalcViewportGeometry();
	EXPECT_EQ(dvl::gsTileVp._vColumns, 8u);
	EXPECT_EQ(dvl::gsTileVp._vRows, 15u);
}

// CalcTileOffset
/*int _vOffsetX; // X-offset in a back buffer
  int _vOffsetY; // Y-offset in a back buffer
  int _vShiftX; // X-shift in a dPiece
  int _vShiftY; // Y-shift in a dPiece*/

TEST(Scrool_rt, calc_tile_offset_original)
{
	/*screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 0);*/
}

TEST(Scrool_rt, calc_tile_offset_original_zoom)
{
	/*screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = false;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 8);*/
}

TEST(Scrool_rt, calc_tile_offset_960_540)
{
	/*screenWidth = 960;
	screenHeight = 540;
	viewportHeight = screenHeight;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 2);*/
}

TEST(Scrool_rt, calc_tile_offset_853_480)
{
	/*screenWidth = 853;
	screenHeight = 480;
	viewportHeight = screenHeight;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 21);
	EXPECT_EQ(y, 0);*/
}

TEST(Scrool_rt, calc_tile_offset_768_480_zoom)
{
	/*screenWidth = 768;
	screenHeight = 480;
	viewportHeight = screenHeight;
	zoomflag = false;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 8);*/
}

// RowsCoveredByPanel

TEST(Scrool_rt, calc_tiles_covered_by_panel_original)
{
	/*screenWidth = 640;
	zoomflag = true;
	EXPECT_EQ(RowsCoveredByPanel(), 0);*/
}

TEST(Scrool_rt, calc_tiles_covered_by_panel_960)
{
	/*screenWidth = 960;
	zoomflag = true;
	EXPECT_EQ(RowsCoveredByPanel(), 4);*/
}

TEST(Scrool_rt, calc_tiles_covered_by_panel_960_zoom)
{
	/*screenWidth = 960;
	zoomflag = false;
	EXPECT_EQ(RowsCoveredByPanel(), 2);*/
}
