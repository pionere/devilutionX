/**
 * @file dx.h
 *
 * Interface of functions setting up the graphics pipeline.
 */
#ifndef __DX_H__
#define __DX_H__

DEVILUTION_BEGIN_NAMESPACE

extern BYTE *gpBuffer;
extern BYTE *gpBufStart;
extern BYTE *gpBufEnd;

void dx_init();
void lock_buf(BYTE idx);
void unlock_buf(BYTE idx);
void dx_cleanup();
void dx_reinit();
void dx_update_palette();
void ClearScreenBuffer();
void BltFast(const SDL_Rect *src_rect, SDL_Rect *dst_rect);
void Blit(SDL_Surface *src, const SDL_Rect *src_rect, SDL_Rect *dst_rect);
void RenderPresent();
void PaletteGetEntries(unsigned dwNumEntries, SDL_Color *lpEntries);

DEVILUTION_END_NAMESPACE

#endif /* __DX_H__ */
