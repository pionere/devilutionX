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

extern BYTE *pPanelText;
extern BYTE *pMedTextCels;

extern const BYTE sfontframe[128];
extern const BYTE mfontframe[128];
extern const BYTE sfontkern[68];
extern const BYTE mfontkern[56];
extern const BYTE gbFontTransTbl[256];

void InitText();
void FreeText();

int GetLargeStringWidth(const char *text);
int GetStringWidth(const char* text);

void PrintChar(int sx, int sy, int nCel, BYTE col);
void PrintString(int x, int y, int endX, const char *pszStr, bool center, BYTE col, int kern);
void PrintGameStr(int x, int y, const char *str, BYTE color);
int PrintLimitedString(int x, int y, const char *text, int limit, BYTE col);
void PrintLargeString(int x, int y, const char *pszStr);

void DrawPentSpn(int x1, int x2, int y);
void DrawPentSpn2(int x1, int x2, int y);
void DrawSinglePentSpn2(int x, int y);

}

DEVILUTION_END_NAMESPACE

#endif /* __TEXT_RENDER_H__ */
