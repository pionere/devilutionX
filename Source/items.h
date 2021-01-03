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
extern ItemStruct item[MAXITEMS + 1];
#ifdef HELLFIRE
extern CornerStoneStruct CornerStone;
extern int auricGold;
extern int MaxGold;
#endif
extern BOOL UniqueItemFlag[NUM_UITEM];
extern int numitems;

void InitItemGFX();
BOOL ItemPlace(int xp, int yp);
void InitItems();
void CalcPlrItemVals(int pnum, BOOL Loadgfx);
void CalcPlrBookVals(int pnum);
void CalcPlrScrolls(int pnum);
void CalcPlrStaff(int pnum);
void ItemStatOk(int pnum, ItemStruct *is);
void CalcPlrInv(int pnum, BOOL Loadgfx);
void CreateBaseItem(ItemStruct *is, int idata);
void GetItemSeed(ItemStruct *is);
void GetGoldSeed(int pnum, ItemStruct *is);
void SetGoldItemValue(ItemStruct *is, int value);
void CreatePlrItems(int pnum);
BOOL ItemSpaceOk(int i, int j);
void SetItemData(int ii, int idata);
void SaveItemPower(int ii, int power, int param1, int param2, int minval, int maxval, int multval);
void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood);
void SetupItem(int ii);
void SpawnUnique(int uid, int x, int y);
void SpawnItem(int mnum, int x, int y, BOOL sendmsg);
void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta);
void CreateRndUseful(int x, int y, BOOL sendmsg);
void CreateTypeItem(int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta);
void RecreateItem(int idx, WORD icreateinfo, int iseed, int ivalue);
void RecreateEar(WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff);
#ifdef HELLFIRE
void SaveCornerStone();
void LoadCornerStone(int x, int y);
#endif
void SpawnQuestItemInArea(int idx, int areasize);
void SpawnQuestItemAt(int idx, int x, int y);
void SpawnQuestItemAround(int idx, int x, int y);
void SpawnRock();
#ifdef HELLFIRE
void SpawnRewardItem(int idx, int xx, int yy);
#endif
void RespawnItem(int ii, BOOL FlipFlag);
void DeleteItem(int ii, int i);
void ProcessItems();
void FreeItemGFX();
void GetItemFrm(int ii);
void GetItemStr(int ii);
void CheckIdentify(int pnum, int cii);
void DoRepair(int pnum, int cii);
void DoRecharge(int pnum, int cii);
#ifdef HELLFIRE
BOOL DoOil(int pnum, int cii);
#endif
void PrintItemPower(char plidx, const ItemStruct *is);
void DrawUniqueInfo();
void DrawItemInfo();
void SpawnSmith(int lvl);
void SpawnPremium(int lvl);
void SetBookLevel(int pnum, ItemStruct *is);
void SpawnWitch(int lvl);
void SpawnBoy(int lvl);
void SpawnHealer(int lvl);
void SpawnStoreGold();
void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue);
int ItemNoFlippy();
void CreateSpellBook(int ispell, int x, int y);
void CreateMagicArmor(int imisc, int icurs, int x, int y);
#ifdef HELLFIRE
void CreateAmulet(int x, int y);
#endif
void CreateMagicWeapon(int itype, int icurs, int x, int y);
BOOL GetItemRecord(int nSeed, WORD wCI, int nIndex);
void SetItemRecord(int nSeed, WORD wCI, int nIndex);
void PutItemRecord(int nSeed, WORD wCI, int nIndex);

/* data */

extern const BYTE ItemCAnimTbl[];
extern const int ItemInvSnds[];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
