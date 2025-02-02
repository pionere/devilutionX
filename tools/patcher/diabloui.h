/**
 * @file diabloui.h
 *
 * Interface of functions from the core game to the DiabloUI.
 */
#ifndef __DIABLOUI_H__
#define __DIABLOUI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void UiInitialize();
void UiDestroy();

/* Defined in title.cpp */
bool UiTitleDialog();
/* Defined in mainmenu.cpp */
int UiMainMenuDialog();
/* Defined in checker.cpp */
void UiCheckerDialog();
/* Defined in merger.cpp */
void UiMergerDialog();
/* Defined in patcher.cpp */
void UiPatcherDialog();
/* Defined in progress.cpp */
bool UiProgressDialog(const char* msg, int (*fnfunc)());

/* Defined in dialogs.cpp */
void UiErrorOkDialog(const char* caption, char* text, bool error = true);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
