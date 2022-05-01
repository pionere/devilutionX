#include "modifier_hints.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include <cstddef>

#include "all.h"
#include "controls/game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

int CalculateTextWidth(const char *s)
{
	return *s == '\0' ? 0 : GetSmallStringWidth(s);
}

int SpaceWidth()
{
	static const int spaceWidth = CalculateTextWidth(" ");
	return spaceWidth;
}

struct CircleMenuHint {
	CircleMenuHint(bool isDpad, const char *top, const char *right, const char *bottom, const char *left)
	    : is_dpad(isDpad)
	    , top(top)
	    , top_w(CalculateTextWidth(top))
	    , right(right)
	    , right_w(CalculateTextWidth(right))
	    , bottom(bottom)
	    , bottom_w(CalculateTextWidth(bottom))
	    , left(left)
	    , left_w(CalculateTextWidth(left))
	    , x_mid(left_w + SpaceWidth() * 5 / 2)
	{
	}

	int Width() const
	{
		return 2 * x_mid;
	}

	bool is_dpad;

	const char *top;
	int top_w;
	const char *right;
	int right_w;
	const char *bottom;
	int bottom_w;
	const char *left;
	int left_w;

	int x_mid;
};

bool IsTopActive(const CircleMenuHint &hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP);
	return IsControllerButtonPressed(ControllerButton_BUTTON_Y);
}

bool IsRightActive(const CircleMenuHint &hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT);
	return IsControllerButtonPressed(ControllerButton_BUTTON_B);
}

bool IsBottomActive(const CircleMenuHint &hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN);
	return IsControllerButtonPressed(ControllerButton_BUTTON_A);
}

bool IsLeftActive(const CircleMenuHint &hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT);
	return IsControllerButtonPressed(ControllerButton_BUTTON_X);
}

text_color CircleMenuHintTextColor(bool active)
{
	return active ? COL_BLUE : COL_GOLD;
}

void DrawCircleMenuHint(const CircleMenuHint &hint, int x, int y)
{
	const int lineHeight = 25;
	x += SCREEN_X;
	y += SCREEN_Y;

	PrintGameStr(x + hint.x_mid - hint.top_w / 2, y, hint.top, CircleMenuHintTextColor(IsTopActive(hint)));
	y += lineHeight;

	PrintGameStr(x, y, hint.left, CircleMenuHintTextColor(IsLeftActive(hint)));
	PrintGameStr(x + hint.left_w + 5 * SpaceWidth(), y, hint.right, CircleMenuHintTextColor(IsRightActive(hint)));
	y += lineHeight;

	PrintGameStr(x + hint.x_mid - hint.bottom_w / 2, y, hint.bottom, CircleMenuHintTextColor(IsBottomActive(hint)));
}

const int CircleMarginX = 16;
const int CirclesTop = 128 + 76;

void DrawStartModifierMenu()
{
	if (!start_modifier_active)
		return;
	static const CircleMenuHint dPad(/*is_dpad=*/true, /*top=*/"Menu", /*right=*/"Inv", /*bottom=*/"Map", /*left=*/"Char");
	static const CircleMenuHint buttons(/*is_dpad=*/false, /*top=*/"", /*right=*/"", /*bottom=*/"Spells", /*left=*/"Quests");
	DrawCircleMenuHint(dPad, PANEL_LEFT + CircleMarginX, SCREEN_HEIGHT - CirclesTop);
	DrawCircleMenuHint(buttons, PANEL_LEFT + PANEL_WIDTH - buttons.Width() - CircleMarginX, SCREEN_HEIGHT - CirclesTop);
}

void DrawSelectModifierMenu()
{
	if (!select_modifier_active)
		return;
	if (dpad_hotkeys) {
		static const CircleMenuHint dPad(/*is_dpad=*/true, /*top=*/"W", /*right=*/"R", /*bottom=*/"E", /*left=*/"Q");
		DrawCircleMenuHint(dPad, PANEL_LEFT + CircleMarginX, SCREEN_HEIGHT - CirclesTop);
	}
	static const CircleMenuHint spells(/*is_dpad=*/false, "W", "R", "E", "Q");
	DrawCircleMenuHint(spells, PANEL_LEFT + PANEL_WIDTH - spells.Width() - CircleMarginX, SCREEN_HEIGHT - CirclesTop);
}

} // namespace

void DrawControllerModifierHints()
{
	DrawStartModifierMenu();
	DrawSelectModifierMenu();
}

DEVILUTION_END_NAMESPACE

#endif
