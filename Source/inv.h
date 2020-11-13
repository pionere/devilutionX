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
extern BOOL drawsbarflag;
extern const InvXY InvRect[NUM_XY_SLOTS];

void FreeInvGFX();
void InitInv();
void DrawInv();
void DrawInvBelt();
BOOL AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct *is);
BOOL GoldAutoPlace(int pnum, ItemStruct *is);
BOOL AutoPlaceInv(int pnum, ItemStruct *is, BOOL saveflag);
BOOL WeaponAutoPlace(int pnum, ItemStruct *is, BOOL saveflag);
int SwapItem(ItemStruct *a, ItemStruct *b);
void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId);
void inv_update_rem_item(int pnum, BYTE iv);
void RemoveInvItem(int pnum, int iv);
#ifdef HELLFIRE
BOOL inv_diablo_to_hellfire(int pnum);
#endif
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvItem();
void CheckInvScrn();
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(int idx, WORD ci, int iseed);
void SyncGetItem(int x, int y, int idx, WORD ci, int iseed);
BOOL CanPut(int x, int y);
BOOL FindItemLocation(int sx, int sy, int *dx, int *dy, int rad);
BOOL DropItem();
void DrawInvMsg(const char *msg);
int InvPutItem(int pnum, int x, int y);
int SyncPutItem(int pnum, int x, int y, ItemStruct *is);
char CheckInvHLight();
void RemoveScroll(int pnum);
BOOL UseScroll();
void UseStaffCharge(int pnum);
BOOL UseStaff();
BOOL UseInvItem(int pnum, int cii);
int CalculateGold(int pnum);

/* data */

extern int AP2x2Tbl[10];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
