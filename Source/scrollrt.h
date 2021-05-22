/**
 * @file scrollrt.h
 *
 * Interface of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#ifndef __SCROLLRT_H__
#define __SCROLLRT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

	/**
 * @brief Shifting the view area along the logical grid
 *        Note: this won't allow you to shift between even and odd rows
 * @param horizontal Shift the screen left or right
 * @param vertical Shift the screen up or down
 */
#define SHIFT_GRID(x, y, horizontal, vertical)	\
{												\
	x += vertical + horizontal;					\
	y += vertical - horizontal;					\
}

extern int light_table_index;
extern bool gbFrameflag;
extern bool gbCelTransparencyActive;
extern bool gbCelFoliageActive;
extern int level_piece_id;

void ClearCursor();
int RowsCoveredByPanel();
void CalcTileOffset(int *offsetX, int *offsetY);
void TilesInView(int *columns, int *rows);
void CalcViewportGeometry();
void ClearScreenBuffer();
#ifdef _DEBUG
void ScrollView();
#endif
void scrollrt_draw_game_screen(bool draw_cursor);
void DrawAndBlit();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SCROLLRT_H__ */
