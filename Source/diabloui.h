/**
 * @file diabloui.h
 *
 * Interface of functions from the core game to the DiabloUI.
 */
#ifndef __DIABLOUI_H__
#define __DIABLOUI_H__

DEVILUTION_BEGIN_NAMESPACE

void UiInitialize();
void UiTitleDialog();
void UiDestroy();

bool UiValidPlayerName(const char *name);
int UiSelHeroDialog(void(*fninfo)(void(*fninfofunc)(_uiheroinfo *)), bool(*fncreate)(_uiheroinfo *), void(*fnremove)(_uiheroinfo *), void(*fnstats)(unsigned int, _uidefaultstats *), char (&name)[16]);
void UiCreditsDialog();
void UiMainMenuDialog(const char *name, int *pdwResult, void(*fnSound)(const char *file), int attractTimeOut);

bool UiProgressDialog(const char *msg, int enable, int(*fnfunc)(), int rate);

bool UiSelectProvider(bool bMulti);
int UiSelectGame(_SNETGAMEDATA *game_data, void (*event_handler)(_SNETEVENT *pEvt));
void UIDisconnectGame();

void UiCreatePlayerDescription(const _uiheroinfo *info, DWORD mode, char (&desc)[128]);
void UiSetupPlayerInfo(const char *infostr, const _uiheroinfo *pInfo, DWORD type);

/* These are defined in fonts.h */
extern bool gbWasFontsInit;
extern void FontsCleanup();

/* Defined in selconn.cpp */
extern int provider;

/* Defined in dialogs.h */
void UiErrorOkDialog(const char *text, const char *caption, bool error = true);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
