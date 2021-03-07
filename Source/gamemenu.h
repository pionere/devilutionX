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
void gamemenu_previous(bool bActivate);
void gamemenu_new_game(bool bActivate);
void gamemenu_quit_game(bool bActivate);
void gamemenu_load_game(bool bActivate);
void gamemenu_save_game(bool bActivate);
void gamemenu_restart_town(bool bActivate);
void gamemenu_settings(bool bActivate);
void gamemenu_music_volume(bool bActivate);
void gamemenu_sound_volume(bool bActivate);
void gamemenu_gamma(bool bActivate);
void gamemenu_speed(bool bActivate);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GAMEMENU_H__ */
