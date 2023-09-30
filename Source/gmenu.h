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

extern CelImageBuf* gpOptbarCel;
extern TMenuItem* gpCurrentMenu;

void gmenu_draw_pause();
void FreeGMenu();
void InitGMenu();
bool gmenu_is_active();
void gmenu_set_items(TMenuItem* pItem, int nItems, void (*gmUpdFunc)());
void gmenu_draw();
void gmenu_presskey(int vkey);
void gmenu_on_mouse_move();
void gmenu_left_mouse(bool isDown);
void gmenu_enable(TMenuItem* pMenuItem, bool enable);
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
void CheckMenuMove();
#endif
void gmenu_update();

/**
 * @brief Set the TMenuItem slider position based on the given value
 */
void gmenu_slider_set(TMenuItem* pItem, int min, int max, int value);

/**
 * @brief Get the current value for the slider
 */
int gmenu_slider_get(TMenuItem* pItem, int min, int max);

/**
 * @brief Set the number of steps for the slider
 */
void gmenu_slider_steps(TMenuItem* pItem, int steps);

#ifdef __cplusplus
}
#endif

inline bool gmenu_is_active()
{
	return gpCurrentMenu != NULL;
}

DEVILUTION_END_NAMESPACE

#endif /* __GMENU_H__ */
