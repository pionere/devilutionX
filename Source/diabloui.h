/**
 * @file diabloui.h
 *
 * Interface of functions from the core game to the DiabloUI.
 */
#ifndef __DIABLOUI_H__
#define __DIABLOUI_H__

#include <vector>

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void UiInitialize();
void UiDestroy();

/* Defined in title.cpp */
bool UiTitleDialog();
/* Defined in selhero.cpp */
int UiSelHeroDialog(unsigned* saveIdx);
/* Defined in settingsmenu.cpp */
void UiSettingsDialog();
/* Defined in credits.cpp */
void UiCreditsDialog();
/* Defined in mainmenu.cpp */
int UiMainMenuDialog();
/* Defined in progress.cpp */
bool UiProgressDialog(const char* msg, int (*fnfunc)());

/* Defined in selgame.cpp */
int UiSelectGame(_uigamedata* game_data);
void UIDisconnectGame();

/* Defined in selconn.cpp */
extern int provider;
bool UiSelectProvider(bool bMulti);

/* Defined in dialogs.cpp */
void UiErrorOkDialog(const char* caption, char* text, bool error = true);

/* Defined in hostgame.cpp */
void UiHostGameDialog();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
