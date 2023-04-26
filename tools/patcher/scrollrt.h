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

extern int light_trn_index;
extern bool gbCelTransparencyActive;

void ClearCursor();
#if DEBUG_MODE
void ScrollView();
#endif
void scrollrt_draw_screen(bool draw_cursor);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SCROLLRT_H__ */
