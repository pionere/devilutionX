/**
 * @file stores.h
 *
 * Interface of functionality for stores and towner dialogs.
 */
#ifndef __STORES_H__
#define __STORES_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/** Currently active store */
extern BYTE stextflag;

/** Current item in store. */
extern ItemStruct storeitem;
/** Current level of the item sold by Wirt */
extern unsigned boylevel;
/** Current item sold by Wirt */
extern ItemStruct boyitem;
/** Normal-Items sold by Griswold */
extern ItemStruct smithitem[SMITH_ITEMS];
/** Base level of current premium items sold by Griswold */
extern int premiumlevel;
/** Number of premium items for sale by Griswold */
extern int numpremium;
/** Premium items sold by Griswold */
extern ItemStruct premiumitems[SMITH_PREMIUM_ITEMS];
/** Items sold by Adria */
extern ItemStruct witchitem[WITCH_ITEMS];
/** Items sold by Pepin */
extern ItemStruct healitem[HEALER_ITEMS];

void InitStoresOnce();
void InitLvlStores();
void StartStore(int s);
void DrawStore();
void STextESC();
void STextUp();
void STextDown();
void STextRight();
void STextLeft();
void STextPageUp();
void STextPageDown();
void SyncStoreCmd(int pnum, int cmd, int ii, int price);
bool TakePlrsMoney(int pnum, int cost);
void STextEnter();
void TryStoreBtnClick();
void ReleaseStoreBtn();

/* rdata */

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __STORES_H__ */
