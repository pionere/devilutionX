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
//bool WeaponAutoPlace(int pnum, ItemStruct *is, bool saveflag);
void InvPasteItem(int pnum, BYTE r);
void InvPasteBeltItem(int pnum, BYTE r);
void InvCutItem(int pnum, BYTE r, bool bShift);
void SyncPlrItemRemove(int pnum, BYTE bLoc);
void SyncPlrStorageRemove(int pnum, int iv);
void SyncPlrSpdBarRemove(int pnum, int iv);
void CheckInvClick(bool bShift);
void CheckBeltClick(bool bShift);
void InvGetItem(int pnum, int ii);
bool AutoGetItem(int pnum, int ii);
bool SyncAutoGetItem(int pnum, int ii);
int FindGetItem(int iseed, WORD wIndex, WORD wCI);
bool CanPut(int x, int y);
bool FindItemLocation(int sx, int sy, int *dx, int *dy, int rad);
void DropItem();
void SyncPutItem(int pnum, int x, int y, bool plrAround);
void SyncSplitGold(int pnum, int cii, int value);
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
