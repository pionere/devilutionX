/**
 * @file dx.h
 *
 * Interface of functions setting up the graphics pipeline.
 */
#ifndef __DX_H__
#define __DX_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/** Back buffer */
extern BYTE* gpBuffer;
/** Upper bound of back buffer. */
extern BYTE* gpBufStart;
/** Lower bound of back buffer. */
extern BYTE* gpBufEnd;
/** The width of the back buffer. */
extern int gnBufferWidth;

void dx_init();
void lock_buf(BYTE idx);
void unlock_buf(BYTE idx);
void dx_cleanup();
#if !FULLSCREEN_ONLY
void ToggleFullscreen();
#endif
void ClearScreenBuffer();
void RedBack();
void BltFast();
void RenderPresent();
void SetSurfaceAndPaletteColors(SDL_Color* colors, int firstcolor, int ncolors);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DX_H__ */
