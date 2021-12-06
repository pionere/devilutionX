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
void gamemenu_settings(bool bActivate);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GAMEMENU_H__ */
