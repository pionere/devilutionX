/**
 * @file diabloui.h
 *
 * Interface of functions from the core game to the DiabloUI.
 */
#ifndef __DIABLOUI_H__
#define __DIABLOUI_H__

DEVILUTION_BEGIN_NAMESPACE

void UiInitialize();
void UiDestroy();

bool UiValidPlayerName(const char *name);

/* Defined in title.cpp */
void UiTitleDialog();
/* Defined in selhero.cpp */
int UiSelHeroDialog(void(*fninfo)(void(*fninfofunc)(_uiheroinfo *)), bool(*fncreate)(_uiheroinfo *), void(*fnremove)(_uiheroinfo *), unsigned* saveIdx);
/* Defined in credits.cpp */
void UiCreditsDialog();
/* Defined in mainmenu.cpp */
int UiMainMenuDialog(const char* name, void(*fnSound)(int sfx, int rndCnt));
/* Defined in progress.cpp */
bool UiProgressDialog(const char *msg, int(*fnfunc)());

/* Defined in selgame.cpp */
int UiSelectGame(SNetGameData* game_data, void (*event_handler)(SNetEvent* pEvt));
void UIDisconnectGame(int reason);

/* These are defined in fonts.h */
extern bool gbWasFontsInit;
extern void FontsCleanup();

/* Defined in selconn.cpp */
extern int provider;
bool UiSelectProvider(bool bMulti);

/* Defined in dialogs.h */
void UiErrorOkDialog(const char *text, const char *caption, bool error = true);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
