/**
 * @file dx.h
 *
 * Interface of functions setting up the graphics pipeline.
 */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

DEVILUTION_BEGIN_NAMESPACE

extern int screenWidth;
extern int screenHeight;
extern int viewportHeight;
extern int widthAlignment;

extern SDL_Window *ghMainWnd;
extern SDL_Palette *palette;
extern SDL_Surface *pal_surface;
extern unsigned int pal_surface_palette_version;

bool SpawnWindow(const char *lpWindowName);

DEVILUTION_END_NAMESPACE

#endif /* __DISPLAY_H__ */
