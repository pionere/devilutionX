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
extern BYTE gbTSpell;   // the spell to cast after the target is selected
extern char gbTSplFrom; // the source of the spell after the target is selected
extern char gbOilFrom;

extern const InvXY InvRect[NUM_XY_SLOTS];
extern const BYTE InvSlotTbl[NUM_XY_SLOTS];

void FreeInvGFX();
void InitInv();
void DrawInv();
void DrawInvBelt();
bool AutoPlaceBelt(int pnum, ItemStruct *is, bool saveflag);
bool AutoPlaceInv(int pnum, ItemStruct *is, bool saveflag);
bool WeaponAutoPlace(int pnum, ItemStruct *is, bool saveflag);
void SyncPlrItemChange(int pnum, BYTE bLoc, int ii);
void SyncPlrItemRemove(int pnum, BYTE bLoc);
void PlrInvItemRemove(int pnum, int iv);
void RemoveInvItem(int pnum, int iv);
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvClick(bool bShift);
void CheckBeltClick(bool bShift);
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(int iseed, WORD wIndex, WORD wCI);
void SyncGetItemAt(int x, int y, int iseed, WORD wIndex, WORD wCI);
void SyncGetItemIdx(int ii);
bool CanPut(int x, int y);
bool FindItemLocation(int sx, int sy, int *dx, int *dy, int rad);
bool DropItem();
int InvPutItem(int pnum, int x, int y, int ii);
int SyncPutItem(int pnum, int x, int y, int ii, bool plrAround);
BYTE CheckInvItem();
BYTE CheckInvBelt();
bool InvUseItem(int cii);
void SyncUseItem(int pnum, int cii);
void CalculateGold(int pnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
