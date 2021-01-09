/**
 * @file gamemenu.h
 *
 * Interface of the in-game menu functions.
 */
#ifndef __GAMEMENU_H__
#define __GAMEMENU_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void gamemenu_on();
void gamemenu_off();
void gamemenu_previous(BOOL bActivate);
void gamemenu_new_game(BOOL bActivate);
void gamemenu_quit_game(BOOL bActivate);
void gamemenu_load_game(BOOL bActivate);
void gamemenu_save_game(BOOL bActivate);
void gamemenu_restart_town(BOOL bActivate);
void gamemenu_settings(BOOL bActivate);
void gamemenu_music_volume(BOOL bActivate);
void gamemenu_sound_volume(BOOL bActivate);
void gamemenu_gamma(BOOL bActivate);
void gamemenu_speed(BOOL bActivate);
void gamemenu_color_cycling(BOOL bActivate);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GAMEMENU_H__ */
