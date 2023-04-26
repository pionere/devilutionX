/**
 * @file cursor.h
 *
 * Interface of cursor tracking functionality.
 */
#ifndef __CURSOR_H__
#define __CURSOR_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern ViewportStruct gsMouseVp;

extern int cursW;
extern int cursH;
extern int pcursmonst;
extern BYTE* pCursCels;
extern BYTE pcurswnd;
extern BYTE pcursinvitem;
extern BYTE pcursitem;
extern BYTE pcursobj;
extern BYTE pcursplr;
extern int pcurstrig;
extern POS32 pcurspos;
extern int pcursicon;

void InitCursorGFX();
void FreeCursorGFX();
void NewCursor(int i);
void InitLevelCursor();
void CheckTownPortal();
void CheckCursMove();

/* rdata */
#define MAX_CURSOR_AREA 8192
extern const int InvItemWidth[(int)CURSOR_FIRSTITEM + (int)NUM_ICURS];
extern const int InvItemHeight[(int)CURSOR_FIRSTITEM + (int)NUM_ICURS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CURSOR_H__ */
