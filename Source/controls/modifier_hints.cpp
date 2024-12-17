#include "modifier_hints.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include <cstddef>

#include "all.h"
#include "engine/render/text_render.h"
#include "controller.h"
#include "game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

int CalculateTextWidth(const char* s)
{
	return *s == '\0' ? 0 : GetSmallStringWidth(s);
}

int SpaceWidth()
{
	static const int spaceWidth = smallFontWidth[0]; // CalculateTextWidth(" ");
	return spaceWidth;
}

struct CircleMenuHint {
	CircleMenuHint(bool isDpad, const char* top, const char* right, const char* bottom, const char* left)
	    : is_dpad(isDpad)
	    , top(top)
	    , right(right)
	    , bottom(bottom)
	    , left(left)
	{
		int left_w = CalculateTextWidth(left); // std::max(CalculateTextWidth(left), CalculateTextWidth(right));
		int x_mid = left_w + SpaceWidth() * 5 / 2u;
		width = 2 * x_mid;
		top_offx = x_mid - CalculateTextWidth(top) / 2u;
		bottom_offx = x_mid - CalculateTextWidth(bottom) / 2u;
		right_offx = left_w + SpaceWidth() * 5;
	}

	bool is_dpad;

	const char* top;
	const char* right;
	const char* bottom;
	const char* left;

	int width;
	int top_offx;
	int right_offx;
	int bottom_offx;
};

bool IsTopActive(const CircleMenuHint& hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP);
	return IsControllerButtonPressed(ControllerButton_BUTTON_FACE_TOP);
}

bool IsRightActive(const CircleMenuHint& hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT);
	return IsControllerButtonPressed(ControllerButton_BUTTON_FACE_RIGHT);
}

bool IsBottomActive(const CircleMenuHint& hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN);
	return IsControllerButtonPressed(ControllerButton_BUTTON_FACE_BOTTOM);
}

bool IsLeftActive(const CircleMenuHint& hint)
{
	if (hint.is_dpad)
		return IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT);
	return IsControllerButtonPressed(ControllerButton_BUTTON_FACE_LEFT);
}

text_color CircleMenuHintTextColor(bool active)
{
	return active ? COL_BLUE : COL_GOLD;
}

void DrawCircleMenuHint(const CircleMenuHint& hint, int x, int y)
{
	const int lineHeight = 25;
	x += SCREEN_X;
	y += SCREEN_Y;

	PrintGameStr(x + hint.top_offx, y, hint.top, CircleMenuHintTextColor(IsTopActive(hint)));
	y += lineHeight;

	PrintGameStr(x, y, hint.left, CircleMenuHintTextColor(IsLeftActive(hint)));
	PrintGameStr(x + hint.right_offx, y, hint.right, CircleMenuHintTextColor(IsRightActive(hint)));
	y += lineHeight;

	PrintGameStr(x + hint.bottom_offx, y, hint.bottom, CircleMenuHintTextColor(IsBottomActive(hint)));
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
	DrawCircleMenuHint(buttons, PANEL_LEFT + PANEL_WIDTH - buttons.width - CircleMarginX, SCREEN_HEIGHT - CirclesTop);
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
	DrawCircleMenuHint(spells, PANEL_LEFT + PANEL_WIDTH - spells.width - CircleMarginX, SCREEN_HEIGHT - CirclesTop);
}

} // namespace

void DrawControllerModifierHints()
{
	DrawStartModifierMenu();
	DrawSelectModifierMenu();
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
