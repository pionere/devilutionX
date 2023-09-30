/**
 * @file inv.h
 *
 * Interface of player inventory.
 */
#ifndef __INV_H__
#define __INV_H__

DEVILUTION_BEGIN_NAMESPACE

#define TWOHAND_WIELD(pp, ii) ((ii)->_iLoc == ILOC_TWOHAND && ((ii)->_itype == ITYPE_BOW || (pp)->_pBaseStr < (ii)->_iMinStr * 4))

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbInvflag;
extern BYTE gbTSpell;   // the spell to cast after the target is selected
extern int8_t gbTSplFrom; // the source of the spell after the target is selected
extern int8_t gbOilFrom;

extern const InvXY InvRect[NUM_XY_SLOTS];
extern const BYTE InvSlotTbl[NUM_XY_SLOTS];

void FreeInvGFX();
void InitInv();
void DrawInv();
void DrawInvBelt();
bool AutoPlaceBelt(int pnum, ItemStruct* is, bool saveflag);
bool AutoPlaceInv(int pnum, ItemStruct* is, bool saveflag);
//bool WeaponAutoPlace(int pnum, ItemStruct* is, bool saveflag);
void InvPasteItem(int pnum, BYTE r);
void InvPasteBeltItem(int pnum, BYTE r);
void InvCutItem(int pnum, BYTE r, bool bShift);
void SyncPlrItemRemove(int pnum, BYTE bLoc);
void SyncPlrStorageRemove(int pnum, int iv);
void CheckInvClick(bool bShift);
void CheckBeltClick(bool bShift);
void InvGetItem(int pnum, int ii);
bool AutoGetItem(int pnum, int ii);
bool SyncAutoGetItem(int pnum, int ii);
int FindGetItem(const PkItemStruct* pkItem);
bool CanPut(int x, int y);
bool FindItemLocation(int sx, int sy, POS32& pos, int rad);
void DropItem();
void SyncPutItem(int pnum, int x, int y, bool flipFlag);
void SyncSplitGold(int pnum, int cii, int value);
BYTE CheckInvItem();
BYTE CheckInvBelt();
bool InvUseItem(int cii);
bool SyncUseItem(int pnum, BYTE cii, BYTE sn);
void CalculateGold(int pnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
