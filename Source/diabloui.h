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
void UiSelHeroMultDialog(BOOL(*fninfo)(BOOL(*fninfofunc)(_uiheroinfo *)), BOOL(*fncreate)(_uiheroinfo *), BOOL(*fnremove)(_uiheroinfo *), void(*fnstats)(unsigned int, _uidefaultstats *), int *dlgresult, BOOL *hero_is_created, char (&name)[16]);
void UiSelHeroSingDialog(BOOL(*fninfo)(BOOL(*fninfofunc)(_uiheroinfo *)), BOOL(*fncreate)(_uiheroinfo *), BOOL(*fnremove)(_uiheroinfo *), void(*fnstats)(unsigned int, _uidefaultstats *), int *dlgresult, char (&name)[16], int *difficulty);
bool UiCreditsDialog();
bool UiMainMenuDialog(const char *name, int *pdwResult, void(*fnSound)(const char *file), int attractTimeOut);

bool UiProgressDialog(const char *msg, int(*fnfunc)());

void InitUICallbacks(_SNETUIDATA &UiData);
int UiSelectGame(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info, _SNETVERSIONDATA *file_info, int *a6);
int UiSelectProvider(_SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info, _SNETVERSIONDATA *file_info);

bool UiCreatePlayerDescription(_uiheroinfo *info, DWORD mode, char (&desc)[128]);
void UiSetupPlayerInfo(char *infostr, _uiheroinfo *pInfo, DWORD type);

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
