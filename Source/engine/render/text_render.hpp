/**
 * @file text_render.hpp
 *
 * Text rendering.
 */
#ifndef __TEXT_RENDER_H__
#define __TEXT_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BYTE* pBigTextCels;

extern const BYTE smallFontFrame[128];
extern const BYTE bhFontFrame[128];
extern const BYTE smallFontWidth[68];
extern const BYTE bigFontWidth[56];
extern const BYTE gbFontTransTbl[256];

void InitText();
void FreeText();

int GetHugeStringWidth(const char* text);
int GetSmallStringWidth(const char* text);

void PrintChar(int sx, int sy, int nCel, BYTE col);
void PrintString(int x, int y, int endX, const char *pszStr, bool center, BYTE col, int kern);
void PrintGameStr(int x, int y, const char *str, BYTE color);
int PrintLimitedString(int x, int y, const char *text, int limit, BYTE col);
void PrintHugeString(int x, int y, const char* pszStr, int light);

void DrawHugePentSpn(int x1, int x2, int y);
void DrawSmallPentSpn(int x1, int x2, int y);
void DrawSingleSmallPentSpn(int x, int y);

}

DEVILUTION_END_NAMESPACE

#endif /* __TEXT_RENDER_H__ */
