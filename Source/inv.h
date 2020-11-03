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
void InvDrawSlotBack(int X, int Y, int W, int H);
void DrawInv();
void DrawInvBelt();
BOOL AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct *is);
BOOL GoldAutoPlace(int pnum);
BOOL WeaponAutoPlace(int pnum);
BOOL AutoPlaceInv(int pnum, ItemStruct *is, BOOL saveflag);
int SwapItem(ItemStruct *a, ItemStruct *b);
void CheckInvPaste(int pnum, int mx, int my);
void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId);
void CheckInvCut(int pnum, int mx, int my);
void inv_update_rem_item(int pnum, BYTE iv);
void RemoveInvItem(int pnum, int iv);
#ifdef HELLFIRE
BOOL inv_diablo_to_hellfire(int pnum);
#endif
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvItem();
void CheckInvScrn();
void CheckBookLevel(int pnum);
void CheckQuestItem(int pnum);
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(int idx, WORD ci, int iseed);
void SyncGetItem(int x, int y, int idx, WORD ci, int iseed);
BOOL CanPut(int x, int y);
BOOL TryInvPut();
void DrawInvMsg(const char *msg);
int InvPutItem(int pnum, int x, int y);
int SyncPutItem(int pnum, int x, int y, int idx, WORD icreateinfo, int iseed, BOOL Id, int dur, int mdur, int ch, int mch, int ivalue, DWORD ibuff
#ifdef HELLFIRE
	, int to_hit, int max_dam, int min_str, int min_mag, int min_dex, int ac
#endif
);
char CheckInvHLight();
void RemoveScroll(int pnum);
BOOL UseScroll();
void UseStaffCharge(int pnum);
BOOL UseStaff();
void StartGoldDrop();
BOOL UseInvItem(int pnum, int cii);
int CalculateGold(int pnum);
BOOL DropItemBeforeTrig();

/* data */

extern int AP2x2Tbl[10];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
