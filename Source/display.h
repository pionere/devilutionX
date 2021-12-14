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

extern bool gbShowFPS;

extern SDL_Window* ghMainWnd;
extern SDL_Palette* back_palette;
extern SDL_Surface* back_surface;
extern unsigned int back_surface_palette_version;

bool SpawnWindow(const char* lpWindowName);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DISPLAY_H__ */
