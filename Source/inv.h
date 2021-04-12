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

extern bool gbInvflag;
extern const InvXY InvRect[NUM_XY_SLOTS];
extern const BYTE InvSlotTbl[NUM_XY_SLOTS];

void FreeInvGFX();
void InitInv();
void DrawInv();
void DrawInvBelt();
bool AutoPlaceInv(int pnum, ItemStruct *is, bool saveflag);
bool WeaponAutoPlace(int pnum, ItemStruct *is, bool saveflag);
void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, bool bId);
void inv_update_rem_item(int pnum, BYTE iv);
void RemoveInvItem(int pnum, int iv);
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvClick();
void CheckBeltClick();
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(WORD idx, WORD ci, int iseed);
void SyncGetItem(int x, int y, int idx, WORD ci, int iseed);
bool CanPut(int x, int y);
bool FindItemLocation(int sx, int sy, int *dx, int *dy, int rad);
bool DropItem();
void DrawInvMsg(const char *msg);
int InvPutItem(int pnum, int x, int y, int ii);
int SyncPutItem(int pnum, int x, int y, int ii, bool plrAround);
BYTE CheckInvItem();
BYTE CheckInvBelt();
bool UseInvItem(int cii);
void CalculateGold(int pnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
