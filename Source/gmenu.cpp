/**
 * @file gmenu.cpp
 *
 * Implementation of the in-game navigation and interaction.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include "controls/axis_direction.h"
#include "controls/controller_motion.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

/** Logo CEL above the menu */
static CelImageBuf* gpLogoCel;
/** Slider button CEL */
static CelImageBuf* gpOptionCel;
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

void FreeGMenu()
{
	MemFreeDbg(gpLogoCel);
	MemFreeDbg(gpOptionCel);
}

void InitGMenu()
{
	gpCurrentMenu = NULL;
	gmUpdateFunc = NULL;
	guCurrItemIdx = 0;
	guCurrentMenuSize = 0;
	_gbMouseNavigation = false;
	assert(gpLogoCel == NULL);
	gpLogoCel = CelLoadImage(LOGO_DATA, LOGO_WIDTH);
	assert(gpOptionCel == NULL);
	gpOptionCel = CelLoadImage("Data\\option.CEL", SLIDER_BUTTON_WIDTH);
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
		if (gpCurrentMenu[n].dwFlags & GMF_ENABLED)
			break;
	}
	if (n != guCurrItemIdx) {
		guCurrItemIdx = n;
		PlaySfx(IS_TITLEMOV);
	}
}

static void gmenu_enter()
{
	if (gpCurrentMenu[guCurrItemIdx].dwFlags & GMF_ENABLED) {
		gpCurrentMenu[guCurrItemIdx].fnMenu(true);
	}
}

static void gmenu_left_right(bool isRight)
{
	TMenuItem* pItem = &gpCurrentMenu[guCurrItemIdx];
	int step, steps;

	if (!(pItem->dwFlags & GMF_SLIDER)) {
		if (isRight) {
			gmenu_enter();
		} else {
			gamemenu_off();
		}
		return;
	}
	if (!(pItem->dwFlags & GMF_ENABLED)) {
		return;
	}

	step = pItem->wMenuParam2;
	steps = pItem->wMenuParam1;
	step += isRight ? 1 : -1;
	if (step < 0 || step > steps) {
		// PlaySfx(IS_TITLEMOV);
		return;
	}
	pItem->wMenuParam2 = step;
	pItem->fnMenu(false);
}

void gmenu_set_items(TMenuItem* pItem, int nItems, void (*gmUpdFunc)())
{
	// pause game if not in the main menu
	if (gbRunGame) {
		diablo_pause_game(pItem != NULL);
	}
	_gbMouseNavigation = false;
	gpCurrentMenu = pItem;
	guCurrentMenuSize = nItems;
	guCurrItemIdx = 0;
	gmUpdateFunc = gmUpdFunc;
	if (gmUpdateFunc != NULL)
		gmUpdateFunc();
	// play select sfx only in-game
	if (gbRunGame)
		PlaySfx(IS_TITLEMOV);
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

static int gmenu_get_lfont(TMenuItem* pItem)
{
	if (pItem->dwFlags & GMF_SLIDER)
		return SLIDER_ROW_WIDTH;
	return GetHugeStringWidth(pItem->pszStr);
}

static TMenuItem* current_menu_item(bool activate)
{
	int i, w;
	TMenuItem* pItem;

	i = MousePos.y - (PANEL_MIDY(GAMEMENU_HEIGHT) + LOGO_HEIGHT + GAMEMENU_HEADER_OFF);
	if (i < 0) {
		return NULL;
	}
	i /= GAMEMENU_ITEM_HEIGHT;
	if (i >= guCurrentMenuSize) {
		return NULL;
	}
	pItem = &gpCurrentMenu[i];
	if (!(pItem->dwFlags & GMF_ENABLED)) {
		return NULL;
	}
	w = gmenu_get_lfont(pItem) / 2u;
	if (abs(MousePos.x - (int)(SCREEN_WIDTH / 2u)) > w)
		return NULL;

	if (activate)
		guCurrItemIdx = i;
	return pItem;
}

static void gmenu_draw_menu_item(int i, int y)
{
	TMenuItem* pItem = &gpCurrentMenu[i];
	TMenuItem* mItem = current_menu_item(false);
	unsigned w, x, nSteps, step, pos;

	w = gmenu_get_lfont(pItem);
	x = PANEL_CENTERX(w);
	PrintHugeString(x, y, pItem->pszStr, COL_GOLD + ((pItem->dwFlags & GMF_ENABLED) ? (pItem == mItem ? 2 : 0) : MAXDARKNESS));
	if (pItem == &gpCurrentMenu[guCurrItemIdx])
		DrawHugePentSpn(x - (FOCUS_HUGE + 6), x + 4 + w, y + 1);
	if (pItem->dwFlags & GMF_SLIDER) {
		x += SLIDER_OFFSET;
		DrawColorTextBox(x, y - 10 - 32 + 1, SLIDER_BOX_WIDTH, 32, COL_GOLD);
		x += SLIDER_BORDER;
		step = pItem->wMenuParam2;
		nSteps = pItem->wMenuParam1;
		pos = step * SLIDER_INNER_WIDTH / nSteps;
		gmenu_draw_rectangle(x, y - 10 - SLIDER_BORDER, pos + SLIDER_BUTTON_WIDTH / 2, SLIDER_BOX_HEIGHT - 2 * SLIDER_BORDER);
		CelDraw(x + pos, y - 10 - SLIDER_BORDER, gpOptionCel, 1);
	}
}

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
void CheckMenuMove()
{
	// assert(gmenu_is_active());
	const AxisDirection move_dir = axisDirRepeater.Get(GetLeftStickOrDpadDirection(true));
	if (move_dir.x != AxisDirectionX_NONE)
		gmenu_left_right(move_dir.x == AxisDirectionX_RIGHT);
	if (move_dir.y != AxisDirectionY_NONE)
		gmenu_up_down(move_dir.y == AxisDirectionY_DOWN);
}
#endif

void gmenu_update()
{
	// assert(gmenu_is_active());
	assert(gmUpdateFunc != NULL);
	gmUpdateFunc();
}

void gmenu_draw()
{
	int nCel, i, y;

#ifdef HELLFIRE
	// nCel = GetAnimationFrame(32, 16);
	nCel = ((SDL_GetTicks() / 32) % 16) + 1;
#else
	nCel = 1;
#endif
	y = PANEL_CENTERY(GAMEMENU_HEIGHT) + LOGO_HEIGHT;
	CelDraw(PANEL_CENTERX(LOGO_WIDTH), y, gpLogoCel, nCel);
	y += GAMEMENU_HEADER_OFF + GAMEMENU_ITEM_HEIGHT;
	for (i = 0; i < guCurrentMenuSize; i++, y += GAMEMENU_ITEM_HEIGHT)
		gmenu_draw_menu_item(i, y);
}

void gmenu_presskey(int vkey)
{
	// assert(gmenu_is_active());
	switch (vkey) {
	case DVL_VK_LBUTTON:
		gmenu_left_mouse(true);
		break;
	case DVL_VK_RETURN:
		gmenu_enter();
		break;
	case DVL_VK_XBUTTON1:
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
}

static void gmenu_mouse_slider()
{
	TMenuItem* pItem;
	int offset;

	offset = MousePos.x - (PANEL_MIDX(SLIDER_ROW_WIDTH) + SLIDER_OFFSET + SLIDER_BORDER + SLIDER_BUTTON_WIDTH / 2);
	if (offset < 0) {
		if (offset < -(SLIDER_BUTTON_WIDTH / 2))
			return;
		offset = 0;
	}
	if (offset > SLIDER_INNER_WIDTH) {
		if (offset > SLIDER_INNER_WIDTH + SLIDER_BUTTON_WIDTH / 2)
			return;
		offset = SLIDER_INNER_WIDTH;
	}
	_gbMouseNavigation = true;
	pItem = &gpCurrentMenu[guCurrItemIdx];
	gmenu_slider_set(pItem, 0, SLIDER_INNER_WIDTH, offset);
	pItem->fnMenu(false);
}

void gmenu_on_mouse_move()
{
	if (_gbMouseNavigation) {
		gmenu_mouse_slider();
		// return TRUE;
	}
	// return FALSE;
}

void gmenu_left_mouse(bool isDown)
{
	TMenuItem* pItem;

	// assert(gmenu_is_active());
	if (!isDown) {
		//if (_gbMouseNavigation) {
			_gbMouseNavigation = false;
		//}
		return;
	}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (sgbControllerActive) {
		gmenu_enter();
		return;
	}
#endif
	pItem = current_menu_item(true);
	if (pItem != NULL) {
		if (pItem->dwFlags & GMF_SLIDER) {
			gmenu_mouse_slider();
		} else {
			pItem->fnMenu(true);
		}
	}
}

void gmenu_enable(TMenuItem* pMenuItem, bool enable)
{
	if (enable)
		pMenuItem->dwFlags |= GMF_ENABLED;
	else
		pMenuItem->dwFlags &= ~GMF_ENABLED;
}

void gmenu_slider_set(TMenuItem* pItem, int min, int max, int value)
{
	int nSteps;

	//assert(pItem != NULL);
	//assert(max > min);
	nSteps = pItem->wMenuParam1;
	pItem->wMenuParam2 = ((max - min) / 2u + (value - min) * nSteps) / (max - min);
}

int gmenu_slider_get(TMenuItem* pItem, int min, int max)
{
	int nSteps, step;

	//assert(pItem != NULL);
	step = pItem->wMenuParam2;
	nSteps = pItem->wMenuParam1;

	return min + step * (max - min) / nSteps;
}

void gmenu_slider_steps(TMenuItem* pItem, int steps)
{
	//assert(pItem != NULL);
	// assert(steps >= 1);
	pItem->wMenuParam1 = steps;
}

DEVILUTION_END_NAMESPACE
