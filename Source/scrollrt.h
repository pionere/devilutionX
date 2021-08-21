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

extern int ViewX;
extern int ViewY;
extern ScrollStruct ScrollInfo;
extern int light_table_index;
extern bool gbFrameflag;
extern bool gbCelTransparencyActive;
extern bool gbCelFoliageActive;
extern int level_piece_id;

void ClearCursor();
void CalcViewportGeometry();
#ifdef _DEBUG
void ScrollView();
#endif
void scrollrt_draw_item(const ItemStruct* is, bool outline, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);
void scrollrt_draw_screen(bool draw_cursor);
void scrollrt_draw_game();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SCROLLRT_H__ */
