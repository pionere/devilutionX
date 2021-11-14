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
extern int numitems;

void InitItemGFX();
void InitItems();
void CalcPlrItemVals(int pnum, bool Loadgfx);
void CalcPlrSpells(int pnum);
void CalcPlrScrolls(int pnum);
void CalcPlrStaff(int pnum);
void ItemStatOk(int pnum, ItemStruct *is);
void CalcPlrInv(int pnum, bool Loadgfx);
void CreateBaseItem(ItemStruct *is, int idata);
void GetItemSeed(ItemStruct *is);
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
void RecreateItem(int iseed, WORD wIndex, WORD wCI, int ivalue);
void SpawnQuestItemInArea(int idx, int areasize);
void SpawnQuestItemAt(int idx, int x, int y, bool sendmsg, bool delta);
void SpawnQuestItemAround(int idx, int x, int y, bool sendmsg/*, bool respawn*/);
void SpawnRock();
#ifdef HELLFIRE
void SpawnRewardItem(int idx, int xx, int yy, bool sendmsg, bool respawn);
void CreateAmulet(WORD wCI, int x, int y, bool sendmsg, bool respawn);
#endif
void RespawnItem(int ii, bool FlipFlag);
void DeleteItem(int ii, int i);
void DeleteItems(int ii);
void ProcessItems();
void FreeItemGFX();
void SyncItemAnim(int ii);
ItemStruct* PlrItem(int pnum, int cii);
void DoAbility(int pnum, char from, BYTE cii);
void DoOil(int pnum, char from, BYTE cii);
const char *ItemName(const ItemStruct* is);
void PrintItemPower(BYTE plidx, const ItemStruct *is);
void DrawUniqueInfo();
void DrawInvItemDetails();
void SpawnSmith(int lvl);
void SpawnPremium(int lvl);
void SpawnWitch(int lvl);
void SpawnBoy(int lvl);
void SpawnHealer(int lvl);
void RecreateTownItem(int ii, int iseed, WORD idx, WORD icreateinfo);
void CreateSpellBook(int ispell, int x, int y);
void CreateMagicItem(int itype, int icurs, int x, int y, bool sendmsg);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
