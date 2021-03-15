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

extern int light_table_index;
extern bool gbFrameflag;
extern bool gbCelTransparencyActive;
extern bool gbCelFoliageActive;
extern int level_piece_id;

void ClearCursor();
void ShiftGrid(int *x, int *y, int horizontal, int vertical);
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
