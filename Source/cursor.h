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

extern int cursW;
extern int cursH;
extern int pcursmonst;
extern BYTE *pCursCels;
#ifdef HELLFIRE
extern BYTE *pCursCels2;
#endif
extern char pcursinvitem;
extern char pcursitem;
extern char pcursobj;
extern char pcursplr;
extern int cursmx;
extern int cursmy;
extern int pcurs;

void InitCursor();
void FreeCursor();
void NewCursor(int i);
void InitLevelCursor();
void CheckTownPortal();
void CheckCursMove();

/* rdata */
#define MAX_CURSOR_AREA		8192
extern const int InvItemWidth[];
extern const int InvItemHeight[];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CURSOR_H__ */
