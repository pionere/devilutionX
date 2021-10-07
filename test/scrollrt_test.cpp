#include <gtest/gtest.h>
#include "all.h"
#include "ui_fwd.h"

using namespace dvl;

// TilesInView

TEST(Scrool_rt, calc_tiles_in_view_original)
{
	screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = true;
	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	EXPECT_EQ(columns, 10);
	EXPECT_EQ(rows, 11);
}

TEST(Scrool_rt, calc_tiles_in_view_original_zoom)
{
	screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = false;
	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	EXPECT_EQ(columns, 5);
	EXPECT_EQ(rows, 6);
}

TEST(Scrool_rt, calc_tiles_in_view_960_540)
{
	screenWidth = 960;
	screenHeight = 540;
	viewportHeight = screenHeight;
	zoomflag = true;
	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	EXPECT_EQ(columns, 15);
	EXPECT_EQ(rows, 17);
}

TEST(Scrool_rt, calc_tiles_in_view_640_512)
{
	screenWidth = 640;
	screenHeight = 512;
	viewportHeight = screenHeight - 128;
	zoomflag = true;
	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	EXPECT_EQ(columns, 10);
	EXPECT_EQ(rows, 12);
}

TEST(Scrool_rt, calc_tiles_in_view_768_480_zoom)
{
	screenWidth = 768;
	screenHeight = 480;
	viewportHeight = screenHeight;
	zoomflag = false;
	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	EXPECT_EQ(columns, 6);
	EXPECT_EQ(rows, 8);
}

// CalcTileOffset

TEST(Scrool_rt, calc_tile_offset_original)
{
	screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 0);
}

TEST(Scrool_rt, calc_tile_offset_original_zoom)
{
	screenWidth = 640;
	screenHeight = 480;
	viewportHeight = screenHeight - 128;
	zoomflag = false;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 8);
}

TEST(Scrool_rt, calc_tile_offset_960_540)
{
	screenWidth = 960;
	screenHeight = 540;
	viewportHeight = screenHeight;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 2);
}

TEST(Scrool_rt, calc_tile_offset_853_480)
{
	screenWidth = 853;
	screenHeight = 480;
	viewportHeight = screenHeight;
	zoomflag = true;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 21);
	EXPECT_EQ(y, 0);
}

TEST(Scrool_rt, calc_tile_offset_768_480_zoom)
{
	screenWidth = 768;
	screenHeight = 480;
	viewportHeight = screenHeight;
	zoomflag = false;
	int x = 0;
	int y = 0;
	CalcTileOffset(&x, &y);
	EXPECT_EQ(x, 0);
	EXPECT_EQ(y, 8);
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
