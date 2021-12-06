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

/* Defined in title.cpp */
void UiTitleDialog();
/* Defined in selhero.cpp */
int UiSelHeroDialog(void(*fninfo)(void(*fninfofunc)(_uiheroinfo*)), int(*fncreate)(_uiheroinfo*), void(*fnremove)(_uiheroinfo*), unsigned* saveIdx);
/* Defined in settingsmenu.cpp */
void UiSettingsDialog();
/* Defined in credits.cpp */
void UiCreditsDialog();
/* Defined in mainmenu.cpp */
int UiMainMenuDialog(const char* name, void(*fnSound)(int sfx, int rndCnt));
/* Defined in progress.cpp */
bool UiProgressDialog(const char *msg, int(*fnfunc)());

/* Defined in selgame.cpp */
int UiSelectGame(SNetGameData* game_data, void (*event_handler)(SNetEvent* pEvt));
void UIDisconnectGame(int reason);

/* Defined in selconn.cpp */
extern int provider;
bool UiSelectProvider(bool bMulti);

/* Defined in dialogs.h */
void UiErrorOkDialog(const char* caption, const char* text, bool error = true);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
