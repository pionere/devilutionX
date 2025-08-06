/**
 * @file inv.h
 *
 * Interface of player inventory.
 */
#ifndef __INV_H__
#define __INV_H__

DEVILUTION_BEGIN_NAMESPACE

#define TWOHAND_WIELD(pp, ii) ((ii)->_iLoc == ILOC_TWOHAND && ((ii)->_itype == ITYPE_BOW || (pp)->_pBaseStr < (ii)->_iMinStr * 4))

static_assert(NUM_INVELEM <= INT8_MAX, "INVIDX_VALID checks only the sign of the INVITEM_-value I.");
static_assert((int8_t)INVITEM_NONE < 0, "INVIDX_VALID checks only the sign of the INVITEM_-value II.");
#define INVIDX_VALID(x) ((int8_t)x >= 0)

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbInvflag;
extern CmdSkillUse gbTSkillUse;   // the spell to cast after the target is selected

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
void InvCutItem(int pnum, BYTE cii, bool bShift);
void SyncPlrItemRemove(int pnum, BYTE cii);
void SyncPlrStorageRemove(int pnum, int cii);
void CheckInvClick(bool altAction);
void CheckBeltClick(bool altAction);
void SyncInvGetItem(int pnum, int ii);
void InvGetItem(int pnum, int ii);
bool SyncAutoGetItem(int pnum, int ii);
bool AutoGetItem(int pnum, int ii);
void SyncSplitGold(int pnum, int cii, int value);
BYTE CheckInvItem();
BYTE CheckInvBelt();
void InvUseItem(int cii);
bool SyncUseItem(int pnum, BYTE cii, BYTE sn);
bool SyncUseMapItem(int pnum, BYTE cii, BYTE mIdx);
void CalculateGold(int pnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
