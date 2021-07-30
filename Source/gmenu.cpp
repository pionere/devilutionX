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

static BYTE *optbar_cel;
static BYTE *option_cel;
static BYTE *sgpLogo;
static bool _gbMouseNavigation;
#ifdef HELLFIRE
static Uint32 guNextLogoAnimTc;
static BYTE gbLogoAnimFrame;
#endif
static void (*gmUpdateFunc)();
TMenuItem *sgpCurrentMenu;
static TMenuItem *sgpCurrItem;
static int _gCurrentMenuSize;

void gmenu_draw_pause()
{
	if (currLvl._dLevelIdx != DLV_TOWN)
		RedBack();
	if (sgpCurrentMenu == NULL) {
		PrintLargeString(316 + PANEL_LEFT, 336, "Pause", 0);
	}
}

void FreeGMenu()
{
	MemFreeDbg(sgpLogo);
	MemFreeDbg(option_cel);
	MemFreeDbg(optbar_cel);
}

void InitGMenu()
{
#ifdef HELLFIRE
	gbLogoAnimFrame = 1;
#endif
	sgpCurrentMenu = NULL;
	sgpCurrItem = NULL;
	gmUpdateFunc = NULL;
	_gCurrentMenuSize = 0;
	_gbMouseNavigation = false;
	sgpLogo = LoadFileInMem(LOGO_DATA);
	option_cel = LoadFileInMem("Data\\option.CEL");
	optbar_cel = LoadFileInMem("Data\\optbar.CEL");
}

static void gmenu_up_down(bool isDown)
{
	int i;

	if (sgpCurrItem == NULL) {
		return;
	}
	_gbMouseNavigation = false;
	i = _gCurrentMenuSize;
	while (i != 0) {
		i--;
		if (isDown) {
			sgpCurrItem++;
			if (sgpCurrItem->fnMenu == NULL)
				sgpCurrItem = &sgpCurrentMenu[0];
		} else {
			if (sgpCurrItem == &sgpCurrentMenu[0])
				sgpCurrItem = &sgpCurrentMenu[_gCurrentMenuSize];
			sgpCurrItem--;
		}
		if (sgpCurrItem->dwFlags & GMENU_ENABLED) {
			if (i != 0)
				PlaySFX(IS_TITLEMOV);
			return;
		}
	}
}

static void gmenu_left_right(bool isRight)
{
	int step, steps;

	if (!(sgpCurrItem->dwFlags & GMENU_SLIDER))
		return;

	step = sgpCurrItem->dwFlags & 0xFFF;
	steps = (sgpCurrItem->dwFlags & 0xFFF000) >> 12;
	step += isRight ? 1 : -1;
	if (step < 0 || step >= steps) {
		// PlaySFX(IS_TITLEMOV);
		return;
	}
	sgpCurrItem->dwFlags &= 0xFFFFF000;
	sgpCurrItem->dwFlags |= step;
	sgpCurrItem->fnMenu(false);
}

void gmenu_set_items(TMenuItem *pItem, void (*gmUpdFunc)())
{
	PauseMode = 0;
	_gbMouseNavigation = false;
	sgpCurrentMenu = pItem;
	gmUpdateFunc = gmUpdFunc;
	if (gmUpdFunc != NULL) {
		gmUpdateFunc();
	}
	sgpCurrItem = NULL;
	_gCurrentMenuSize = 0;
	if (pItem != NULL) {
		sgpCurrItem = &pItem[0];
		while (pItem[_gCurrentMenuSize].fnMenu != NULL)
			_gCurrentMenuSize++;
	}
	PlaySFX(IS_TITLEMOV);
}

static void gmenu_clear_buffer(int x, int y, int width, int height)
{
	int i;
	BYTE *pBuf;

	pBuf = gpBuffer + BUFFER_WIDTH * y + x;
	for (i = height; i != 0; i--) {
		memset(pBuf, 205, width);
		pBuf -= BUFFER_WIDTH;
	}
}

static int gmenu_get_lfont(TMenuItem *pItem)
{
	if (pItem->dwFlags & GMENU_SLIDER)
		return 490;
	return GetLargeStringWidth(pItem->pszStr);
}

static void gmenu_draw_menu_item(TMenuItem *pItem, int y)
{
	DWORD w, x, nSteps, step, pos;

	w = gmenu_get_lfont(pItem);
	if (pItem->dwFlags & GMENU_SLIDER) {
		x = 16 + w / 2 + SCREEN_X;
		CelDraw(x + PANEL_LEFT, y - 10, optbar_cel, 1, 287);
		step = pItem->dwFlags & 0xFFF;
		nSteps = (pItem->dwFlags & 0xFFF000) >> 12;
		pos = step * 256 / nSteps;
		gmenu_clear_buffer(x + 2 + PANEL_LEFT, y - 12, pos + 13, 28);
		CelDraw(x + 2 + pos + PANEL_LEFT, y - 12, option_cel, 1, 27);
	}
	x = SCREEN_WIDTH / 2 - w / 2 + SCREEN_X;
	PrintLargeString(x, y, pItem->pszStr, (pItem->dwFlags & GMENU_ENABLED) ? 0 : LIGHTMAX);
	if (pItem == sgpCurrItem) {
		DrawPentSpn(x - 54, x + 4 + w, y + 1);
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
	int nCel, y;
	TMenuItem *i;

	assert(sgpCurrentMenu != NULL);
	GameMenuMove();
	if (gmUpdateFunc != NULL)
		gmUpdateFunc();
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
	CelDraw((SCREEN_WIDTH - LOGO_WIDTH) / 2 + SCREEN_X, 102 + SCREEN_Y + UI_OFFSET_Y, sgpLogo, nCel, LOGO_WIDTH);
	y = 160 + SCREEN_Y + UI_OFFSET_Y;
	for (i = sgpCurrentMenu; i->fnMenu != NULL; i++, y += 45)
		gmenu_draw_menu_item(i, y);
}

bool gmenu_presskeys(int vkey)
{
	switch (vkey) {
	case DVL_VK_LBUTTON:
		return gmenu_left_mouse(true);
	case DVL_VK_RETURN:
		if (sgpCurrItem->dwFlags & GMENU_ENABLED) {
			sgpCurrItem->fnMenu(true);
		}
		break;
	case DVL_VK_ESCAPE:
	case DVL_VK_SPACE:
		gmenu_set_items(NULL, NULL);
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

static bool gmenu_get_mouse_slider(int *plOffset)
{
	int offset;

	offset = MouseX - (PANEL_LEFT + 282);
	if (offset < 0) {
		*plOffset = 0;
		return false;
	}
	if (offset > 256) {
		*plOffset = 256;
		return false;
	}
	*plOffset = offset;
	return true;
}

void gmenu_on_mouse_move()
{
	int offset;

	if (!_gbMouseNavigation)
		return; // FALSE;
	gmenu_get_mouse_slider(&offset);

	gmenu_slider_set(sgpCurrItem, 0, 256, offset);
	sgpCurrItem->fnMenu(false);
	// return TRUE;
}

bool gmenu_left_mouse(bool isDown)
{
	TMenuItem *pItem;
	int i, w, dummy;

	if (!isDown) {
		if (_gbMouseNavigation) {
			_gbMouseNavigation = false;
			return true;
		} else {
			return false;
		}
	}

	if (sgpCurrentMenu == NULL) {
		return false;
	}
	if (MouseY >= PANEL_TOP) {
		return false;
	}
	i = MouseY - (117 + UI_OFFSET_Y);
	if (i < 0) {
		return true;
	}
	i /= 45;
	if (i >= _gCurrentMenuSize) {
		return true;
	}
	pItem = &sgpCurrentMenu[i];
	if (!(pItem->dwFlags & GMENU_ENABLED)) {
		return true;
	}
	w = gmenu_get_lfont(pItem) / 2;
	if (MouseX < SCREEN_WIDTH / 2 - w) {
		return true;
	}
	if (MouseX > SCREEN_WIDTH / 2 + w) {
		return true;
	}
	sgpCurrItem = pItem;
	if (pItem->dwFlags & GMENU_SLIDER) {
		_gbMouseNavigation = gmenu_get_mouse_slider(&dummy);
		gmenu_on_mouse_move();
	} else {
		sgpCurrItem->fnMenu(true);
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

/**
 * @brief Set the TMenuItem slider position based on the given value
 */
void gmenu_slider_set(TMenuItem *pItem, int min, int max, int value)
{
	int nSteps;

	assert(pItem != NULL);
	nSteps = (pItem->dwFlags & 0xFFF000) >> 12;
	pItem->dwFlags &= 0xFFFFF000;
	pItem->dwFlags |= ((max - min - 1) + (value - min) * nSteps) / (max - min);
}

/**
 * @brief Get the current value for the slider
 */
int gmenu_slider_get(TMenuItem *pItem, int min, int max)
{
	int nSteps, step;

	step = pItem->dwFlags & 0xFFF;
	nSteps = (pItem->dwFlags & 0xFFF000) >> 12;

	return min + step * (max - min) / nSteps;
}

/**
 * @brief Set the number of steps for the slider
 */
void gmenu_slider_steps(TMenuItem *pItem, int steps)
{
	// assert(steps >= 2);
	pItem->dwFlags &= 0xFF000FFF;
	pItem->dwFlags |= ((steps - 1) << 12) & 0xFFF000;
}

DEVILUTION_END_NAMESPACE
