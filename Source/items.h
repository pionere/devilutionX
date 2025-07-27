/**
 * @file items.h
 *
 * Interface of item functionality.
 */
#ifndef __ITEMS_H__
#define __ITEMS_H__

DEVILUTION_BEGIN_NAMESPACE

static_assert((int)ITYPE_SWORD + 1 == (int)ITYPE_AXE, "ITYPE_DURABLE check requires a specific ITYPE order I.");
static_assert((int)ITYPE_AXE + 1 == (int)ITYPE_BOW, "ITYPE_DURABLE check requires a specific ITYPE order II.");
static_assert((int)ITYPE_BOW + 1 == (int)ITYPE_MACE, "ITYPE_DURABLE check requires a specific ITYPE order III.");
static_assert((int)ITYPE_MACE + 1 == (int)ITYPE_STAFF, "ITYPE_DURABLE check requires a specific ITYPE order IV.");
static_assert((int)ITYPE_STAFF + 1 == (int)ITYPE_SHIELD, "ITYPE_DURABLE check requires a specific ITYPE order V.");
static_assert((int)ITYPE_SHIELD + 1 == (int)ITYPE_HELM, "ITYPE_DURABLE check requires a specific ITYPE order VI.");
static_assert((int)ITYPE_HELM + 1 == (int)ITYPE_LARMOR, "ITYPE_DURABLE check requires a specific ITYPE order VII.");
static_assert((int)ITYPE_LARMOR + 1 == (int)ITYPE_MARMOR, "ITYPE_DURABLE check requires a specific ITYPE order VIII.");
static_assert((int)ITYPE_MARMOR + 1 == (int)ITYPE_HARMOR, "ITYPE_DURABLE check requires a specific ITYPE order IX.");
#define ITYPE_DURABLE(itype) (itype >= ITYPE_SWORD && itype <= ITYPE_HARMOR)

#ifdef __cplusplus
extern "C" {
#endif

extern int itemactive[MAXITEMS];
extern ItemStruct items[MAXITEMS + 1];
extern int numitems;

void InitItemGFX();
void FreeItemGFX();
void InitLvlItems();
void InitItems();
void CalcPlrItemVals(int pnum, bool Loadgfx);
void CalcPlrSpells(int pnum);
void CalcPlrScrolls(int pnum);
void CalcPlrCharges(int pnum);
void ItemStatOk(int pnum, ItemStruct* is);
void CalcPlrInv(int pnum, bool Loadgfx);
void CreateBaseItem(ItemStruct* is, int idata);
void GetItemSeed(ItemStruct* is);
void SetGoldItemValue(ItemStruct* is, int value);
void CreatePlrItems(int pnum);
bool ItemSpaceOk(int x, int y);
void SetItemData(int ii, int idata);
void SetItemSData(ItemStruct* is, int idata);
void SpawnUnique(int uid, int x, int y, int mode);
void SpawnMonItem(int mnum, int x, int y, bool sendmsg);
void CreateRndItem(int x, int y, unsigned quality, int mode);
void SpawnRndUseful(int x, int y, bool sendmsg);
void CreateTypeItem(int x, int y, unsigned quality, int itype, int imisc, int mode);
void RecreateItem(int32_t iseed, uint16_t wIndex, uint16_t wCI);
void PlaceQuestItemInArea(int idx, int areasize);
void CreateQuestItemAt(int idx, int x, int y, int mode);
void SpawnQuestItemAt(int idx, int x, int y, int mode);
void PlaceRock();
#ifdef HELLFIRE
void SpawnAmulet(uint16_t wCI, int x, int y /*, bool sendmsg*/);
#endif
void RespawnItem(int ii, bool FlipFlag);
void DeleteItem(int ii);
void ProcessItems();
void SyncItemAnim(int ii);
int FindGetItem(const PkItemStruct* pkItem);
void DropItem();
void SyncPutItem(int pnum, int x, int y, const ItemStruct* is, bool flipFlag);
ItemStruct* PlrItem(int pnum, int cii);
void DoAbility(int pnum, int8_t from, BYTE cii);
void DoOil(int pnum, int8_t from, BYTE cii);
const char* ItemName(const ItemStruct* is);
int ItemColor(const ItemStruct* is);
void PrintItemPower(BYTE plidx, const ItemStruct* is);
void DrawInvItemDetails();
void SpawnSmith(unsigned lvl);
void SpawnPremium(unsigned lvl);
void SpawnWitch(unsigned lvl);
void SpawnBoy(unsigned lvl);
void SpawnHealer(unsigned lvl);
void SpawnSpellBook(int ispell, int x, int y, bool sendmsg);
void SpawnMagicItem(int itype, int icurs, int x, int y, bool sendmsg);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMS_H__ */
