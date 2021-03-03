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
int UiSelHeroDialog(bool bMulti, BOOL(*fninfo)(BOOL(*fninfofunc)(_uiheroinfo *)), BOOL(*fncreate)(_uiheroinfo *), BOOL(*fnremove)(_uiheroinfo *), void(*fnstats)(unsigned int, _uidefaultstats *), char (&name)[16]);
void UiCreditsDialog();
void UiMainMenuDialog(const char *name, int *pdwResult, void(*fnSound)(const char *file), int attractTimeOut);

BOOL UiProgressDialog(const char *msg, int enable, int(*fnfunc)(), int rate);

bool UiSelectGame(_SNETGAMEDATA *game_data, int *playerId);
bool UiSelectProvider();

void UiCreatePlayerDescription(_uiheroinfo *info, DWORD mode, char (&desc)[128]);
void UiSetupPlayerInfo(char *infostr, _uiheroinfo *pInfo, DWORD type);

/* These are defined in fonts.h */
extern BOOL was_fonts_init;
extern void FontsCleanup();

/* Defined in selconn.cpp */
extern int provider;

DEVILUTION_END_NAMESPACE

#endif /* __DIABLOUI_H__ */
