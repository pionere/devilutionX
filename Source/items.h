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
extern BOOL uitemflag;
extern int itemavail[MAXITEMS];
extern ItemGetRecordStruct itemrecord[MAXITEMS];
extern ItemStruct item[MAXITEMS + 1];
#ifdef HELLFIRE
extern CornerStoneStruct CornerStone;
#endif
extern BOOL UniqueItemFlag[NUM_UITEM];
#ifdef HELLFIRE
extern int auricGold;
#endif
extern int numitems;

#ifdef HELLFIRE
int items_4231CA(int i);
int items_423230(int i);
int items_423296(int i);
int items_4232FC(int i);
int items_423362(int i);
int items_4233C8(int i);
int items_42342E(int i);
int items_4234B2(int i);
int items_423518(int i);
int items_42357E(int i);
#endif
void InitItemGFX();
BOOL ItemPlace(int xp, int yp);
void InitItems();
void CalcPlrItemVals(int pnum, BOOL Loadgfx);
void CalcPlrScrolls(int pnum);
void CalcPlrStaff(int pnum);
void ItemStatOk(int pnum, ItemStruct *is);
void CalcPlrInv(int pnum, BOOL Loadgfx);
void SetItemData(ItemStruct *is, int idata);
void GetItemSeed(ItemStruct *is);
void GetGoldSeed(int pnum, ItemStruct *is);
void SetGoldItemValue(ItemStruct *is, int value);
void CreatePlrItems(int pnum);
BOOL ItemSpaceOk(int i, int j);
void GetSuperItemLoc(int x, int y, int *xx, int *yy);
void GetItemAttrs(int ii, int idata, int lvl);
void SaveItemPower(int ii, int power, int param1, int param2, int minval, int maxval, int multval);
void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood);
void SetupItem(int ii);
void SpawnUnique(int uid, int x, int y);
void SpawnItem(int mnum, int x, int y, BOOL sendmsg);
void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta);
void CreateRndUseful(int pnum, int x, int y, BOOL sendmsg);
void CreateTypeItem(int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta);
void RecreateItem(int idx, WORD icreateinfo, int iseed, int ivalue);
void RecreateEar(WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff);
#ifdef HELLFIRE
void SaveCornerStone();
void LoadCornerStone(int x, int y);
#endif
void SpawnQuestItem(int itemid, int x, int y, int randarea, int selflag);
void SpawnRock();
#ifdef HELLFIRE
void SpawnRewardItem(int itemid, int xx, int yy);
void SpawnMapOfDoom(int xx, int yy);
void SpawnRuneBomb(int xx, int yy);
void SpawnTheodore(int xx, int yy);
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
void PrintItemDetails(const ItemStruct *is);
void UseItem(int pnum, int Mid, int spl);
void SpawnSmith(int lvl);
void SpawnPremium(int lvl);
void SetBookLevel(int pnum, ItemStruct *is);
void SpawnWitch(int lvl);
void SpawnBoy(int lvl);
void SpawnHealer(int lvl);
void SpawnStoreGold();
void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue);
int ItemNoFlippy();
void CreateSpellBook(int x, int y, int ispell, BOOL sendmsg, BOOL delta);
void CreateMagicArmor(int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta);
#ifdef HELLFIRE
void CreateAmulet(int x, int y, int curlv, BOOL sendmsg, BOOL delta);
#endif
void CreateMagicWeapon(int x, int y, int itype, int icurs, BOOL sendmsg, BOOL delta);
BOOL GetItemRecord(int nSeed, WORD wCI, int nIndex);
void SetItemRecord(int nSeed, WORD wCI, int nIndex);
void PutItemRecord(int nSeed, WORD wCI, int nIndex);

/* data */

#ifdef HELLFIRE
extern int MaxGold;
#endif

extern const BYTE ItemCAnimTbl[];
extern const int ItemInvSnds[];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
