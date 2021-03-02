/**
 * @file mainmenu.h
 *
 * Interface of functions for interacting with the main menu.
 */
#ifndef __MAINMENU_H__
#define __MAINMENU_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern char gszHero[16];

void mainmenu_change_name(int arg1, int arg2, int arg3, int arg4, char *name_1, char *name_2);
bool mainmenu_select_hero_dialog(const _SNETPROGRAMDATA *client_info);
void mainmenu_loop();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MAINMENU_H__ */
