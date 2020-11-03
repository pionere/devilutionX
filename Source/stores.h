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

extern int stextup;
extern int storenumh;
extern int stextlhold;
extern ItemStruct boyitem;
extern int stextshold;
extern ItemStruct premiumitem[SMITH_PREMIUM_ITEMS];
extern BYTE *pSTextBoxCels;
extern int premiumlevel;
extern int talker;
extern STextStruct stext[STORE_LINES];
extern BOOL stextsize;
extern int stextsmax;
extern int gossipstart;
extern ItemStruct witchitem[WITCH_ITEMS];
extern BOOL stextscrl;
extern int numpremium;
extern ItemStruct healitem[HEALER_ITEMS];
extern ItemStruct golditem;
extern BYTE *pSTextSlidCels;
extern int stextvhold;
extern int stextsel;
extern char stextscrldbtn;
extern int gossipend;
extern BYTE *pSPentSpn2Cels;
extern int stextsidx;
extern int boylevel;
extern ItemStruct smithitem[SMITH_ITEMS];
extern int stextdown;
extern char stextscrlubtn;
extern char stextflag;

void InitStores();
int PentSpn2Spin();
void SetupTownStores();
void FreeStoreMem();
void DrawSTextBack();
void PrintSString(int x, int y, BOOL cjustflag, const char *str, char col, int val);
void DrawSLine(int y);
void DrawSSlider(int y1, int y2);
void DrawSTextHelp();
void ClearSText(int s, int e);
void AddSLine(int y);
void AddSTextVal(int y, int val);
void OffsetSTextY(int y, int yo);
void AddSText(int x, int y, BOOL j, const char *str, char clr, BOOL sel);
void S_StartSmith();
void S_ScrollSBuy();
void PrintStoreItem(const ItemStruct *is, int l, char iclr);
void S_StartSBuy();
void S_ScrollSPBuy();
BOOL S_StartSPBuy();
BOOL SmithSellOk(const ItemStruct *is);
void S_ScrollSSell();
void S_StartSSell();
BOOL SmithRepairOk(const ItemStruct *is);
void S_StartSRepair();
void AddStoreHoldRepair(const ItemStruct *is, int i);
void S_StartWitch();
void S_ScrollWBuy();
void S_StartWBuy();
BOOL WitchSellOk(const ItemStruct *is);
void S_StartWSell();
BOOL WitchRechargeOk(const ItemStruct *is);
void AddStoreHoldRecharge(const ItemStruct *is, int i);
void S_StartWRecharge();
void S_StartNoMoney();
void S_StartNoRoom();
void S_StartConfirm();
void S_StartBoy();
void S_StartBBoy();
void S_StartHealer();
void S_ScrollHBuy();
void S_StartHBuy();
void S_StartStory();
BOOL IdItemOk(const ItemStruct *is);
void AddStoreHoldId(const ItemStruct *is, int i);
void S_StartSIdentify();
void S_StartIdShow();
void S_StartTalk();
void S_StartTavern();
void S_StartBarMaid();
void S_StartDrunk();
void StartStore(char s);
void DrawSText();
void STextESC();
void STextUp();
void STextDown();
void STextPrior();
void STextNext();
void S_SmithEnter();
void TakePlrsMoney(int cost);
void SmithBuyItem();
void S_SBuyEnter();
void SmithBuyPItem();
void S_SPBuyEnter();
BOOL StoreGoldFit(int idx);
void PlaceStoreGold(int v);
void StoreSellItem();
void S_SSellEnter();
void SmithRepairItem();
void S_SRepairEnter();
void S_WitchEnter();
void WitchBuyItem();
void S_WBuyEnter();
void S_WSellEnter();
void WitchRechargeItem();
void S_WRechargeEnter();
void S_BoyEnter();
void BoyBuyItem();
void HealerBuyItem();
void S_BBuyEnter();
void StoryIdItem();
void S_ConfirmEnter();
void S_HealerEnter();
void S_HBuyEnter();
void S_StoryEnter();
void S_SIDEnter();
void S_TalkEnter();
void S_TavernEnter();
void S_BarmaidEnter();
void S_DrunkEnter();
void STextEnter();
void CheckStoreBtn();
void ReleaseStoreBtn();

/* rdata */

extern const char *const talkname[9];
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __STORES_H__ */
