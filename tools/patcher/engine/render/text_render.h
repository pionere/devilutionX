/**
 * @file text_render.h
 *
 * Text rendering.
 */
#ifndef __TEXT_RENDER_H__
#define __TEXT_RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define FONT_KERN_SMALL 1
#define FONT_KERN_BIG   2
#define FONT_KERN_HUGE  2

#ifdef __cplusplus
extern "C" {
#endif

extern CelImageBuf* pBigTextCels;

extern const BYTE smallFontWidth[107];
extern const BYTE bigFontWidth[107];
extern const BYTE gbStdFontFrame[256];

void InitText();
void FreeText();

int GetHugeStringWidth(const char* text);
int GetBigStringWidth(const char* text);
int GetSmallStringWidth(const char* text);

int PrintSmallChar(int sx, int sy, BYTE text, BYTE col);
int PrintBigChar(int sx, int sy, BYTE text, BYTE col);
int PrintHugeChar(int sx, int sy, BYTE text, BYTE col);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TEXT_RENDER_H__ */
