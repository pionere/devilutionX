/**
 * @file items.h
 *
 * Interface of item functionality.
 */
#ifndef __ITEMS_H__
#define __ITEMS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int itemactive[MAXITEMS];
extern int itemavail[MAXITEMS];
extern ItemStruct items[MAXITEMS + 1];
extern BOOL UniqueItemFlags[NUM_UITEM];
extern int numitems;

void InitItemGFX();
void InitItems();
void CalcPlrItemVals(int pnum, bool Loadgfx);
void CalcPlrSpells(int pnum);
void CalcPlrBookVals(int pnum);
void CalcPlrScrolls(int pnum);
void CalcPlrStaff(int pnum);
void ItemStatOk(int pnum, ItemStruct *is);
void CalcPlrInv(int pnum, bool Loadgfx);
void CreateBaseItem(ItemStruct *is, int idata);
void GetItemSeed(ItemStruct *is);
void GetGoldSeed(int pnum, ItemStruct *is);
void SetGoldItemValue(ItemStruct *is, int value);
void CreatePlrItems(int pnum);
bool ItemSpaceOk(int x, int y);
void SetItemData(int ii, int idata);
void SetupItem(int ii);
void SpawnUnique(int uid, int x, int y, bool sendmsg, bool respawn);
void SpawnItem(int mnum, int x, int y, bool sendmsg);
void CreateRndItem(int x, int y, bool onlygood, bool sendmsg, bool delta);
void CreateRndUseful(int x, int y, bool sendmsg, bool delta);
void CreateTypeItem(int x, int y, bool onlygood, int itype, int imisc, bool sendmsg, bool delta);
void RecreateItem(int idx, WORD icreateinfo, int iseed, int ivalue);
void RecreateEar(WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff);
void SpawnQuestItemInArea(int idx, int areasize);
void SpawnQuestItemAt(int idx, int x, int y, bool sendmsg, bool delta);
void SpawnQuestItemAround(int idx, int x, int y, bool sendmsg, bool respawn);
void SpawnRock();
#ifdef HELLFIRE
void SpawnRewardItem(int idx, int xx, int yy, bool sendmsg, bool respawn);
void CreateAmulet(int x, int y, bool sendmsg, bool respawn);
#endif
void RespawnItem(int ii, bool FlipFlag);
void DeleteItem(int ii, int i);
void ProcessItems();
void FreeItemGFX();
void GetItemFrm(int ii);
void CheckIdentify(int pnum, int cii);
void DoRepair(int pnum, int cii);
void DoRecharge(int pnum, int cii);
#ifdef HELLFIRE
void DoWhittle(int pnum, int cii);
#endif
void DoOil(int pnum, int from, int cii);
void PrintItemPower(BYTE plidx, const ItemStruct *is);
void DrawUniqueInfo();
void DrawInvItemDetails();
void SpawnSmith(int lvl);
void SpawnPremium(int lvl);
void SpawnWitch(int lvl);
void SpawnBoy(int lvl);
void SpawnHealer(int lvl);
void SpawnStoreGold();
void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed);
int ItemNoFlippy();
void CreateSpellBook(int ispell, int x, int y);
void CreateMagicItem(int itype, int icurs, int x, int y, bool sendmsg);
bool GetItemRecord(int nSeed, WORD wCI, int nIndex);
void SetItemRecord(int nSeed, WORD wCI, int nIndex);
void PutItemRecord(int nSeed, WORD wCI, int nIndex);

/* data */

extern const BYTE ItemCAnimTbl[NUM_ICURS];
extern const int ItemInvSnds[ITEMTYPES];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
