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
void gamemenu_draw();
void gamemenu_enter(int submenu);
void gamemenu_on_mouse_move();
void gamemenu_left_mouse(bool isDown);
void gamemenu_presskey(int vkey);
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
void CheckMenuMove();
#endif
void gamemenu_settings(bool bActivate);
void gamemenu_update();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GAMEMENU_H__ */
