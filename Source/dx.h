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
void ToggleFullscreen();
void ClearScreenBuffer();
void RedBack();
void trans_rect(int sx, int sy, int width, int height);
void BltFast();
void RenderPresent();

DEVILUTION_END_NAMESPACE

#endif /* __DX_H__ */
