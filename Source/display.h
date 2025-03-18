/**
 * @file dx.h
 *
 * Interface of functions setting up the graphics pipeline.
 */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int screenWidth;
extern int screenHeight;
//extern int viewportHeight;

extern bool gbWndActive;

extern SDL_Window* ghMainWnd;

void SpawnWindow();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DISPLAY_H__ */
