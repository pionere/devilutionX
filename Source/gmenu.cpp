/**
 * @file gmenu.cpp
 *
 * Implementation of the in-game navigation and interaction.
 */
#include "all.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#include "controls/axis_direction.h"
#include "controls/controller_motion.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#define MENU_ITEM_HEIGHT	45
#define SLIDER_ROW_WIDTH	490
#define SLIDER_OFFSET		186
#define SLIDER_BORDER		2
#define SLIDER_STEPS		256
#define SLIDER_BUTTON_WIDTH	27

/** Logo CEL above the menu */
static BYTE* gpLogoCel;
#ifdef HELLFIRE
/** The next tick to increment the frame of the logo animation. */
static Uint32 guNextLogoAnimTc;
/** The current frame of the logo animation. */
static BYTE gbLogoAnimFrame;
#endif
/** Slider CEL */
static BYTE* gpOptbarCel;
/** Slider button CEL */
static BYTE* gpOptionCel;
/** Speficifies whether the mouse is pressed while navigating the menu. */
static bool _gbMouseNavigation;
/** The array of the current menu items. */
TMenuItem* gpCurrentMenu;
/** The size of the menu item array. */
static int guCurrentMenuSize;
/** The update function to enable/disable the menu items. */
static void (*gmUpdateFunc)();
/** The index of currently selected menu item. */
static int guCurrItemIdx;

void gmenu_draw_pause()
{
	int x, light;

	if (!gmenu_is_active()) {
		x = SCREEN_X + SCREEN_WIDTH / 2 - GetLargeStringWidth("Pause") / 2;
		light = (SDL_GetTicks() / 256) % 4;
		PrintLargeString(x, SCREEN_Y + SCREEN_HEIGHT / 2 - TILE_HEIGHT * 2, "Pause", light);
	}
}

void FreeGMenu()
{
	MemFreeDbg(gpLogoCel);
	MemFreeDbg(gpOptionCel);
	MemFreeDbg(gpOptbarCel);
}

void InitGMenu()
{
#ifdef HELLFIRE
	gbLogoAnimFrame = 1;
#endif
	gpCurrentMenu = NULL;
	gmUpdateFunc = NULL;
	guCurrItemIdx = 0;
	guCurrentMenuSize = 0;
	_gbMouseNavigation = false;
	assert(gpLogoCel == NULL);
	gpLogoCel = LoadFileInMem(LOGO_DATA);
	assert(gpOptionCel == NULL);
	gpOptionCel = LoadFileInMem("Data\\option.CEL");
	assert(gpOptbarCel == NULL);
	gpOptbarCel = LoadFileInMem("Data\\optbar.CEL");
}

static void gmenu_up_down(bool isDown)
{
	int i, n;

	assert(gmenu_is_active());

	_gbMouseNavigation = false;
	n = guCurrItemIdx;
	for (i = 0; i < guCurrentMenuSize; i++) {
		if (isDown) {
			if (++n >= guCurrentMenuSize)
				n = 0;
		} else {
			if (--n < 0)
				n = guCurrentMenuSize - 1;
		}
		if (gpCurrentMenu[n].dwFlags & GMENU_ENABLED)
			break;
	}
	if (n != guCurrItemIdx) {
		guCurrItemIdx = n;
		PlaySFX(IS_TITLEMOV);
	}
}

static void gmenu_left_right(bool isRight)
{
	TMenuItem* pItem = &gpCurrentMenu[guCurrItemIdx];
	int step, steps;

	if ((pItem->dwFlags & (GMENU_SLIDER | GMENU_ENABLED)) != (GMENU_SLIDER | GMENU_ENABLED))
		return;

	step = pItem->wMenuParam2;
	steps = pItem->wMenuParam1;
	step += isRight ? 1 : -1;
	if (step < 0 || step > steps) {
		// PlaySFX(IS_TITLEMOV);
		return;
	}
	pItem->wMenuParam2 = step;
	pItem->fnMenu(false);
}

void gmenu_set_items(TMenuItem* pItem, int nItems, void (*gmUpdFunc)())
{
	if (!IsMultiGame) {
		gbGamePaused = pItem != NULL;
		sound_pause(gbGamePaused);
		//diablo_pause_game();
	}
	_gbMouseNavigation = false;
	gpCurrentMenu = pItem;
	guCurrentMenuSize = nItems;
	guCurrItemIdx = 0;
	gmUpdateFunc = gmUpdFunc;
	PlaySFX(IS_TITLEMOV);
}

static void gmenu_draw_rectangle(int x, int y, int width, int height)
{
	int i;
	BYTE* dst;

	dst = &gpBuffer[x + BUFFER_WIDTH * y];
	for (i = height; i != 0; i--) {
		memset(dst, PAL16_YELLOW + 13, width);
		dst -= BUFFER_WIDTH;
	}
}

static int gmenu_get_lfont(TMenuItem *pItem)
{
	if (pItem->dwFlags & GMENU_SLIDER)
		return SLIDER_ROW_WIDTH;
	return GetLargeStringWidth(pItem->pszStr);
}

static void gmenu_draw_menu_item(int i, int y)
{
	TMenuItem* pItem = &gpCurrentMenu[i];
	unsigned w, x, nSteps, step, pos;

	w = gmenu_get_lfont(pItem);
	x = SCREEN_X + (SCREEN_WIDTH - w) / 2;
	PrintLargeString(x, y, pItem->pszStr, (pItem->dwFlags & GMENU_ENABLED) ? 0 : LIGHTMAX);
	if (pItem == &gpCurrentMenu[guCurrItemIdx])
		DrawPentSpn(x - 54, x + 4 + w, y + 1);
	if (pItem->dwFlags & GMENU_SLIDER) {
		x += SLIDER_OFFSET;
		CelDraw(x, y - 10, gpOptbarCel, 1, 287);
		x += SLIDER_BORDER;
		step = pItem->wMenuParam2;
		nSteps = pItem->wMenuParam1;
		pos = step * SLIDER_STEPS / nSteps;
		gmenu_draw_rectangle(x, y - 12, pos + SLIDER_BUTTON_WIDTH / 2, 28);
		CelDraw(x + pos, y - 12, gpOptionCel, 1, SLIDER_BUTTON_WIDTH);
	}
}

static void GameMenuMove()
{
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	static AxisDirectionRepeater repeater;
	const AxisDirection move_dir = repeater.Get(GetLeftStickOrDpadDirection());
	if (move_dir.x != AxisDirectionX_NONE)
		gmenu_left_right(move_dir.x == AxisDirectionX_RIGHT);
	if (move_dir.y != AxisDirectionY_NONE)
		gmenu_up_down(move_dir.y == AxisDirectionY_DOWN);
#endif
}

void gmenu_draw()
{
	int nCel, i, y;

	assert(gmenu_is_active());
	assert(gmUpdateFunc != NULL);
	gmUpdateFunc();
	GameMenuMove();
#ifdef HELLFIRE
	Uint32 currTc = SDL_GetTicks();
	if (currTc > guNextLogoAnimTc) {
		guNextLogoAnimTc = currTc + 25;
		gbLogoAnimFrame++;
		if (gbLogoAnimFrame > 16)
			gbLogoAnimFrame = 1;
	}
	nCel = gbLogoAnimFrame;
#else
	nCel = 1;
#endif
	y = 102 + SCREEN_Y + UI_OFFSET_Y;
	CelDraw(SCREEN_X + (SCREEN_WIDTH - LOGO_WIDTH) / 2, y, gpLogoCel, nCel, LOGO_WIDTH);
	y += 58;
	for (i = 0; i < guCurrentMenuSize; i++, y += MENU_ITEM_HEIGHT)
		gmenu_draw_menu_item(i, y);
}

bool gmenu_presskeys(int vkey)
{
	assert(gmUpdateFunc != NULL);
	gmUpdateFunc();

	switch (vkey) {
	case DVL_VK_LBUTTON:
		return gmenu_left_mouse(true);
	case DVL_VK_RETURN:
		if (gpCurrentMenu[guCurrItemIdx].dwFlags & GMENU_ENABLED) {
			gpCurrentMenu[guCurrItemIdx].fnMenu(true);
		}
		break;
	case DVL_VK_ESCAPE:
	case DVL_VK_SPACE:
		gamemenu_off(); // TODO: add gmCloseFunc?
		break;
	case DVL_VK_LEFT:
		gmenu_left_right(false);
		break;
	case DVL_VK_RIGHT:
		gmenu_left_right(true);
		break;
	case DVL_VK_UP:
		gmenu_up_down(false);
		break;
	case DVL_VK_DOWN:
		gmenu_up_down(true);
		break;
	}
	return true;
}

static void gmenu_mouse_slider()
{
	TMenuItem* pItem;
	int offset;

	offset = MouseX - (SCREEN_WIDTH / 2 - SLIDER_ROW_WIDTH / 2 + SLIDER_OFFSET + SLIDER_BORDER + SLIDER_BUTTON_WIDTH / 2);
	if (offset < 0) {
		if (offset < -(SLIDER_BUTTON_WIDTH / 2))
			return;
		offset = 0;
	}
	if (offset > SLIDER_STEPS) {
		if (offset > SLIDER_STEPS + SLIDER_BUTTON_WIDTH / 2)
			return;
		offset = SLIDER_STEPS;
	}
	_gbMouseNavigation = true;
	pItem = &gpCurrentMenu[guCurrItemIdx];
	gmenu_slider_set(pItem, 0, SLIDER_STEPS, offset);
	pItem->fnMenu(false);
}

void gmenu_on_mouse_move()
{
	if (!_gbMouseNavigation)
		return; // FALSE;
	gmenu_mouse_slider();
	// return TRUE;
}

bool gmenu_left_mouse(bool isDown)
{
	TMenuItem *pItem;
	int i, w;

	assert(gmenu_is_active());
	if (!isDown) {
		if (_gbMouseNavigation) {
			_gbMouseNavigation = false;
			return true;
		} else {
			return false;
		}
	}

	if (MouseY >= PANEL_TOP) {
		return false;
	}
	i = MouseY - (117 + UI_OFFSET_Y);
	if (i < 0) {
		return true;
	}
	i /= MENU_ITEM_HEIGHT;
	if (i >= guCurrentMenuSize) {
		return true;
	}
	pItem = &gpCurrentMenu[i];
	if (!(pItem->dwFlags & GMENU_ENABLED)) {
		return true;
	}
	w = gmenu_get_lfont(pItem) / 2;
	if (abs(MouseX - SCREEN_WIDTH / 2) > w)
		return true;
	guCurrItemIdx = i;
	if (pItem->dwFlags & GMENU_SLIDER) {
		gmenu_mouse_slider();
	} else {
		pItem->fnMenu(true);
	}
	return true;
}

void gmenu_enable(TMenuItem *pMenuItem, bool enable)
{
	if (enable)
		pMenuItem->dwFlags |= GMENU_ENABLED;
	else
		pMenuItem->dwFlags &= ~GMENU_ENABLED;
}

void gmenu_slider_set(TMenuItem *pItem, int min, int max, int value)
{
	int nSteps;

	//assert(pItem != NULL);
	nSteps = pItem->wMenuParam1;
	pItem->wMenuParam2 = ((max - min) / 2 + (value - min) * nSteps) / (max - min);
}

int gmenu_slider_get(TMenuItem *pItem, int min, int max)
{
	int nSteps, step;

	//assert(pItem != NULL);
	step = pItem->wMenuParam2;
	nSteps = pItem->wMenuParam1;

	return min + step * (max - min) / nSteps;
}

void gmenu_slider_steps(TMenuItem *pItem, int steps)
{
	//assert(pItem != NULL);
	// assert(steps >= 1);
	pItem->wMenuParam1 = steps;
}

DEVILUTION_END_NAMESPACE
