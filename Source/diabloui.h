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

bool UiProgressDialog(const char *msg, int(*fnfunc)());

bool UiSelectProvider(bool bMulti);
int UiSelectGame(SNetGameData* game_data, void (*event_handler)(SNetEvent* pEvt));
void UIDisconnectGame(int reason);

/* These are defined in fonts.h */
extern bool gbWasFontsInit;
extern void FontsCleanup();

/* Defined in selconn.cpp */
extern int provider;

/* Defined in dialogs.h */
void UiErrorOkDialog(const char *text, const char *caption, bool error = true);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
