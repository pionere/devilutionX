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

/** Shop frame graphics */
extern BYTE *pSTextBoxCels;
/** Scrollbar graphics */
extern BYTE *pSTextSlidCels;
/** Small text selection cursor */
extern BYTE *pSPentSpn2Cels;
/** Currently active store */
extern char stextflag;

/** Temporary item used to generate gold piles by various function */
extern ItemStruct golditem;
/** Current level of the item sold by Wirt */
extern int boylevel;
/** Current item sold by Wirt */
extern ItemStruct boyitem;
/** Normal-Items sold by Griswold */
extern ItemStruct smithitem[SMITH_ITEMS];
/** Base level of current premium items sold by Griswold */
extern int premiumlevel;
/** Number of premium items for sale by Griswold */
extern int numpremium;
/** Premium items sold by Griswold */
extern ItemStruct premiumitem[SMITH_PREMIUM_ITEMS];
/** Items sold by Adria */
extern ItemStruct witchitem[WITCH_ITEMS];
/** Items sold by Pepin */
extern ItemStruct healitem[HEALER_ITEMS];

void InitStores();
int PentSpn2Spin();
void SetupTownStores();
void FreeStoreMem();
void PrintSString(int x, int y, bool cjustflag, const char *str, char col, int val);
void DrawSLine(int y);
void DrawSTextHelp();
void ClearSText(int s, int e);
void StartStore(char s);
void DrawSText();
void STextESC();
void STextUp();
void STextDown();
void STextPrior();
void STextNext();
void TakePlrsMoney(int cost);
void STextEnter();
void CheckStoreBtn();
void ReleaseStoreBtn();

/* rdata */

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __STORES_H__ */
