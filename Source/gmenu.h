/**
 * @file gmenu.h
 *
 * Interface of the in-game navigation and interaction.
 */
#ifndef __GMENU_H__
#define __GMENU_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern TMenuItem *sgpCurrentMenu;

void gmenu_draw_pause();
void FreeGMenu();
void gmenu_init_menu();
bool gmenu_is_active();
void gmenu_set_items(TMenuItem *pItem, void (*gmUpdFunc)(TMenuItem *));
void gmenu_draw();
bool gmenu_presskeys(int vkey);
void gmenu_on_mouse_move();
bool gmenu_left_mouse(bool isDown);
void gmenu_enable(TMenuItem *pMenuItem, bool enable);
void gmenu_slider_set(TMenuItem *pItem, int min, int max, int gamma);
int gmenu_slider_get(TMenuItem *pItem, int min, int max);
void gmenu_slider_steps(TMenuItem *pItem, int dwTicks);

#ifdef __cplusplus
}
#endif

inline bool gmenu_is_active() {
	return sgpCurrentMenu != NULL;
}

DEVILUTION_END_NAMESPACE

#endif /* __GMENU_H__ */
