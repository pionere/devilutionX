#include "menu_controls.h"

#include "DiabloUI/diabloui.h"
#include "controls/axis_direction.h"
#include "controls/plrctrls.h"
#include "controls/controller_motion.h"
#include "controls/remap_keyboard.h"
#include "utils/sdl_compat.h"

DEVILUTION_BEGIN_NAMESPACE

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
MenuAction GetMenuHeldUpDownAction()
{
	static AxisDirectionRepeater repeater;
	const AxisDirection dir = repeater.Get(GetLeftStickOrDpadDirection());
	switch (dir.y) {
	case AxisDirectionY_UP:
		return MenuAction_UP;
	case AxisDirectionY_DOWN:
		return MenuAction_DOWN;
	default:
		return MenuAction_NONE;
	}
}
#endif

MenuAction GetMenuAction(const SDL_Event &event)
{
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	const ControllerButtonEvent ctrlEvent = ToControllerButtonEvent(event);

	if (ProcessControllerMotion(event, ctrlEvent)) {
		sgbControllerActive = true;
		return GetMenuHeldUpDownAction();
	}

	if (ctrlEvent.button != ControllerButton_NONE)
		sgbControllerActive = true;

	if (!ctrlEvent.up) {
		switch (ctrlEvent.button) {
		case ControllerButton_IGNORE:
			return MenuAction_NONE;
		case ControllerButton_BUTTON_B: // Right button
		case ControllerButton_BUTTON_START:
			return MenuAction_SELECT;
		case ControllerButton_BUTTON_BACK:
		case ControllerButton_BUTTON_A: // Bottom button
			return MenuAction_BACK;
		case ControllerButton_BUTTON_X: // Left button
			return MenuAction_DELETE;
		case ControllerButton_BUTTON_DPAD_UP:
		case ControllerButton_BUTTON_DPAD_DOWN:
			return GetMenuHeldUpDownAction();
		case ControllerButton_BUTTON_DPAD_LEFT:
			return MenuAction_LEFT;
		case ControllerButton_BUTTON_DPAD_RIGHT:
			return MenuAction_RIGHT;
		case ControllerButton_BUTTON_LEFTSHOULDER:
			return MenuAction_PAGE_UP;
		case ControllerButton_BUTTON_RIGHTSHOULDER:
			return MenuAction_PAGE_DOWN;
		default:
			break;
		}
	}
#endif

#if HAS_KBCTRL == 0
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (event.type >= SDL_KEYDOWN && event.type < SDL_JOYAXISMOTION)
		sgbControllerActive = false;
#endif

	if (event.type == SDL_KEYDOWN) {
		auto sym = event.key.keysym.sym;
		remap_keyboard_key(&sym);
		switch (sym) {
		case SDLK_UP:
			return MenuAction_UP;
		case SDLK_DOWN:
			return MenuAction_DOWN;
		case SDLK_TAB:
			return (SDL_GetModState() & KMOD_SHIFT) ? MenuAction_UP : MenuAction_DOWN;
		case SDLK_PAGEUP:
			return MenuAction_PAGE_UP;
		case SDLK_PAGEDOWN:
			return MenuAction_PAGE_DOWN;
		case SDLK_RETURN: {
			if (!GetAsyncKeyState(DVL_VK_MENU)) {
				return MenuAction_SELECT;
			}
			break;
		}
		case SDLK_KP_ENTER:
			return MenuAction_SELECT;
		case SDLK_SPACE:
			if (gUiEditField == NULL) {
				return MenuAction_SELECT;
			}
			break;
		case SDLK_DELETE:
			return MenuAction_DELETE;
		case SDLK_LEFT:
			return MenuAction_LEFT;
		case SDLK_RIGHT:
			return MenuAction_RIGHT;
		case SDLK_ESCAPE:
			return MenuAction_BACK;
		default:
			break;
		}
	}
#endif

	if (event.type == SDL_MOUSEBUTTONDOWN) {
		switch (event.button.button) {
		case SDL_BUTTON_X1:
#if !SDL_VERSION_ATLEAST(2, 0, 0)
		case 8:
#endif
			return MenuAction_BACK;
		default:
			break;
		}
	}

	return MenuAction_NONE;
}

DEVILUTION_END_NAMESPACE
