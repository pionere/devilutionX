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
extern ItemStruct curruitem;
extern ItemGetRecordStruct itemrecord[MAXITEMS];
extern ItemStruct item[MAXITEMS + 1];
extern BOOL itemhold[3][3];
#ifdef HELLFIRE
extern CornerStoneStruct CornerStone;
#endif
extern BYTE *itemanims[ITEMTYPES];
extern BOOL UniqueItemFlag[128];
#ifdef HELLFIRE
extern int auricGold;
#endif
extern int numitems;
extern int gnNumGetRecords;

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
void SpawnNote();
#endif
void InitItemGFX();
BOOL ItemPlace(int xp, int yp);
void AddInitItems();
void InitItems();
void CalcPlrItemVals(int pnum, BOOL Loadgfx);
void CalcPlrScrolls(int pnum);
void CalcPlrStaff(int pnum);
void CalcSelfItems(int pnum);
void CalcPlrItemMin(int pnum);
void ItemStatOk(int pnum, ItemStruct *is);
void CalcPlrBookVals(int pnum);
void CalcPlrInv(int pnum, BOOL Loadgfx);
void SetPlrHandItem(ItemStruct *is, int idata);
void GetPlrHandSeed(ItemStruct *is);
void GetGoldSeed(int pnum, ItemStruct *is);
void SetGoldItemValue(ItemStruct *is, int value);
void CreatePlrItems(int pnum);
BOOL ItemSpaceOk(int i, int j);
BOOL GetItemSpace(int x, int y, char ii);
void GetSuperItemSpace(int x, int y, char ii);
void GetSuperItemLoc(int x, int y, int *xx, int *yy);
void CalcItemValue(int ii);
void GetBookSpell(int ii, int lvl);
void GetStaffPower(int ii, int lvl, int bs, BOOL onlygood);
void GetStaffSpell(int ii, int lvl, BOOL onlygood);
void GetItemAttrs(int ii, int idata, int lvl);
int RndPL(int param1, int param2);
int PLVal(int pv, int p1, int p2, int minv, int maxv);
void SaveItemPower(int ii, int power, int param1, int param2, int minval, int maxval, int multval);
void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood);
void GetItemBonus(int ii, int idata, int minlvl, int maxlvl, BOOL onlygood, BOOLEAN allowspells);
void SetupItem(int ii);
int RndItem(int lvl);
int RndUItem(int lvl);
int RndAllItems(int lvl);
int RndTypeItems(int itype, int imid, int lvl);
int CheckUnique(int ii, int lvl, int uper, BOOL recreate);
void GetUniqueItem(int ii, int uid);
void SpawnUnique(int uid, int x, int y);
void ItemRndDur(int ii);
void SetupAllItems(int ii, int idx, int iseed, int lvl, int uper, BOOL onlygood, BOOL recreate, BOOL pregen);
void SpawnItem(int mnum, int x, int y, BOOL sendmsg);
void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta);
void SetupAllUseful(int ii, int iseed, int lvl);
void CreateRndUseful(int pnum, int x, int y, BOOL sendmsg);
void CreateTypeItem(int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta);
void RecreateItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue);
void RecreateEar(int ii, WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff);
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
void ItemDoppel();
void ProcessItems();
void FreeItemGFX();
void GetItemFrm(int ii);
void GetItemStr(int ii);
void CheckIdentify(int pnum, int cii);
void DoRepair(int pnum, int cii);
void RepairItem(ItemStruct *is, int lvl);
void DoRecharge(int pnum, int cii);
#ifdef HELLFIRE
void DoOil(int pnum, int cii);
#endif
void RechargeItem(ItemStruct *is, int r);
void PrintItemOil(char IDidx);
void PrintItemPower(char plidx, ItemStruct *is);
void DrawUTextBack();
void PrintUString(int x, int y, BOOL cjustflag, const char *str, int col);
void DrawULine(int y);
void DrawUniqueInfo();
void PrintItemMisc(ItemStruct *is);
void PrintItemDetails(ItemStruct *is);
void PrintItemDur(ItemStruct *is);
void UseItem(int pnum, int Mid, int spl);
BOOL SmithItemOk(int i);
int RndSmithItem(int lvl);
void BubbleSwapItem(ItemStruct *a, ItemStruct *b);
void SortSmith();
void SpawnSmith(int lvl);
BOOL PremiumItemOk(int i);
int RndPremiumItem(int minlvl, int maxlvl);
void SpawnOnePremium(int i, int plvl);
void SpawnPremium(int lvl);
BOOL WitchItemOk(int i);
int RndWitchItem(int lvl);
void SortWitch();
void WitchBookLevel(int ii);
void SpawnWitch(int lvl);
int RndBoyItem(int lvl);
void SpawnBoy(int lvl);
BOOL HealerItemOk(int i);
int RndHealerItem(int lvl);
void SortHealer();
void SpawnHealer(int lvl);
void SpawnStoreGold();
void RecreateSmithItem(int ii, int idx, int lvl, int iseed);
void RecreatePremiumItem(int ii, int idx, int plvl, int iseed);
void RecreateBoyItem(int ii, int idx, int lvl, int iseed);
void RecreateWitchItem(int ii, int idx, int lvl, int iseed);
void RecreateHealerItem(int ii, int idx, int lvl, int iseed);
void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue);
void RecalcStoreStats();
int ItemNoFlippy();
void CreateSpellBook(int x, int y, int ispell, BOOL sendmsg, BOOL delta);
void CreateMagicArmor(int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta);
#ifdef HELLFIRE
void CreateAmulet(int x, int y, int curlv, BOOL sendmsg, BOOL delta);
#endif
void CreateMagicWeapon(int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta);
BOOL GetItemRecord(int nSeed, WORD wCI, int nIndex);
void NextItemRecord(int i);
void SetItemRecord(int nSeed, WORD wCI, int nIndex);
void PutItemRecord(int nSeed, WORD wCI, int nIndex);

/* data */

#ifdef HELLFIRE
extern int OilLevels[10];
extern int OilValues[10];
extern int OilMagic[10];
extern char OilNames[10][25];
extern int MaxGold;
#endif

extern BYTE ItemCAnimTbl[];
extern const char *const ItemDropNames[];
extern BYTE ItemAnimLs[];
extern int ItemDropSnds[];
extern int ItemInvSnds[];
extern int idoppely;
extern int premiumlvladd[6];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
