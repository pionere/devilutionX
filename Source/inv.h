/**
 * @file inv.h
 *
 * Interface of player inventory.
 */
#ifndef __INV_H__
#define __INV_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL invflag;
extern const InvXY InvRect[NUM_XY_SLOTS];

void FreeInvGFX();
void InitInv();
void DrawInv();
void DrawInvBelt();
BOOL AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct *is);
BOOL GoldAutoPlace(int pnum, ItemStruct *is);
BOOL AutoPlaceInv(int pnum, ItemStruct *is, BOOL saveflag);
BOOL WeaponAutoPlace(int pnum, ItemStruct *is, BOOL saveflag);
void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId);
void inv_update_rem_item(int pnum, BYTE iv);
void RemoveInvItem(int pnum, int iv);
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvItem();
void CheckInvScrn();
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(WORD idx, WORD ci, int iseed);
void SyncGetItem(int x, int y, int idx, WORD ci, int iseed);
BOOL CanPut(int x, int y);
BOOL FindItemLocation(int sx, int sy, int *dx, int *dy, int rad);
BOOL DropItem();
void DrawInvMsg(const char *msg);
int InvPutItem(int pnum, int x, int y);
int SyncPutItem(int pnum, int x, int y, ItemStruct *is);
char CheckInvHLight();
BOOL UseInvItem(int cii);
void CalculateGold(int pnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
