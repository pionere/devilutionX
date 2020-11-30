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

BOOL UiValidPlayerName(const char *name);
BOOL UiSelHeroMultDialog(BOOL(*fninfo)(BOOL(*fninfofunc)(_uiheroinfo *)), BOOL(*fncreate)(_uiheroinfo *), BOOL(*fnremove)(_uiheroinfo *), BOOL(*fnstats)(unsigned int, _uidefaultstats *), int *dlgresult, BOOL *hero_is_created, char (&name)[16]);
BOOL UiSelHeroSingDialog(BOOL(*fninfo)(BOOL(*fninfofunc)(_uiheroinfo *)), BOOL(*fncreate)(_uiheroinfo *), BOOL(*fnremove)(_uiheroinfo *), BOOL(*fnstats)(unsigned int, _uidefaultstats *), int *dlgresult, char (&name)[16], int *difficulty);
BOOL UiCreditsDialog(int a1);
BOOL UiMainMenuDialog(const char *name, int *pdwResult, void(*fnSound)(const char *file), int attractTimeOut);

BOOL UiProgressDialog(const char *msg, int enable, int(*fnfunc)(), int rate);

void InitUICallbacks(_SNETUIDATA &UiData);
int UiSelectGame(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info, _SNETVERSIONDATA *file_info, int *a6);
int UiSelectProvider(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info, _SNETVERSIONDATA *file_info, int *type);

BOOL UiCreatePlayerDescription(_uiheroinfo *info, DWORD mode, char (&desc)[128]);
void UiSetupPlayerInfo(char *infostr, _uiheroinfo *pInfo, DWORD type);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
