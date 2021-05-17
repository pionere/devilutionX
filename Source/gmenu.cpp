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
static BYTE *PentSpin_cel;
static BYTE *BigTGold_cel;
static BYTE *option_cel;
static BYTE *sgpLogo;
static bool _gbMouseNavigation;
#ifdef HELLFIRE
static int LogoAnim_tick;
static BYTE LogoAnim_frame;
#endif
static void (*gmUpdateFunc)();
TMenuItem *sgpCurrentMenu;
static TMenuItem *sgpCurrItem;
static int _gCurrentMenuSize;

/** Maps from font index to bigtgold.cel frame number. */
const BYTE lfontframe[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 37, 49, 38, 0, 39, 40, 47,
	42, 43, 41, 45, 52, 44, 53, 55, 36, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 51, 50,
	0, 46, 0, 54, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 42, 0, 43, 0, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 20, 0, 21, 0, 0
};

/** Maps from bigtgold.cel frame number to character width. */
const BYTE lfontkern[] = {
	18, 33, 21, 26, 28, 19, 19, 26, 25, 11,
	12, 25, 19, 34, 28, 32, 20, 32, 28, 20,
	28, 36, 35, 46, 33, 33, 24, 11, 23, 22,
	22, 21, 22, 21, 21, 21, 32, 10, 20, 36,
	31, 17, 13, 12, 13, 18, 16, 11, 20, 21,
	11, 10, 12, 11, 21, 23
};

static void gmenu_print_text(int x, int y, const char *pszStr)
{
	BYTE c;

	while (*pszStr != '\0') {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = lfontframe[c];
		if (c != 0)
			CelDrawLight(x, y, BigTGold_cel, c, 46, NULL);
		x += lfontkern[c] + 2;
	}
}

void gmenu_draw_pause()
{
	if (currLvl._dLevelIdx != DLV_TOWN)
		RedBack();
	if (sgpCurrentMenu == NULL) {
		light_table_index = 0;
		gmenu_print_text(316 + PANEL_LEFT, 336, "Pause");
	}
}

void FreeGMenu()
{
	MemFreeDbg(sgpLogo);
	MemFreeDbg(BigTGold_cel);
	MemFreeDbg(PentSpin_cel);
	MemFreeDbg(option_cel);
	MemFreeDbg(optbar_cel);
}

void InitGMenu()
{
#ifdef HELLFIRE
	LogoAnim_frame = 1;
#endif
	sgpCurrentMenu = NULL;
	sgpCurrItem = NULL;
	gmUpdateFunc = NULL;
	_gCurrentMenuSize = 0;
	_gbMouseNavigation = false;
	sgpLogo = LoadFileInMem(LOGO_DATA);
	BigTGold_cel = LoadFileInMem("Data\\BigTGold.CEL");
	PentSpin_cel = LoadFileInMem("Data\\PentSpin.CEL");
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
	const char *text;
	int i;
	BYTE c;

	if (pItem->dwFlags & GMENU_SLIDER)
		return 490;
	text = pItem->pszStr;
	i = 0;
	while (*text != '\0') {
		c = gbFontTransTbl[(BYTE)*text++];
		i += lfontkern[lfontframe[c]] + 2;
	}
	return i - 2;
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
	light_table_index = (pItem->dwFlags & GMENU_ENABLED) ? 0 : 15;
	gmenu_print_text(x, y, pItem->pszStr);
	if (pItem == sgpCurrItem) {
		CelDraw(x - 54, y + 1, PentSpin_cel, PentSpn2Spin(), 48);
		CelDraw(x + 4 + w, y + 1, PentSpin_cel, PentSpn2Spin(), 48);
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
		DWORD ticks = SDL_GetTicks();
		if ((int)(ticks - LogoAnim_tick) > 25) {
			LogoAnim_frame++;
			if (LogoAnim_frame > 16)
				LogoAnim_frame = 1;
			LogoAnim_tick = ticks;
		}
		nCel = LogoAnim_frame;
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
