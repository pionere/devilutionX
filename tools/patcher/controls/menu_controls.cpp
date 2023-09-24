#include "menu_controls.h"

#include "controller.h"
#include "controller_motion.h"
#include "axis_direction.h"
//#include "game_controls.h"
#include "touch.h"

#include "DiabloUI/diabloui.h"
#include "remap_keyboard.h"

DEVILUTION_BEGIN_NAMESPACE

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
MenuAction GetMenuHeldUpDownAction()
{
	const AxisDirection dir = axisDirRepeater.Get(GetLeftStickOrDpadDirection());
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

MenuAction GetMenuAction(SDL_Event& event)
{
#if HAS_TOUCHPAD
	handle_touch(&event);
#endif

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	HandleControllerAddedOrRemovedEvent(event);

	const ControllerButtonEvent ctrlEvent = ToControllerButtonEvent(event);

	if (ProcessControllerMotion(event)) {
		sgbControllerActive = true;
		return GetMenuHeldUpDownAction();
	}
#if HAS_DPAD
	if (/*!dpad_hotkeys &&*/ SimulateRightStickWithDpad(ctrlEvent)) {
		sgbControllerActive = true;
		return GetMenuHeldUpDownAction();
	}
#endif

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

#if !HAS_KBCTRL
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#if (HAS_TOUCHPAD || HAS_DPAD) && !defined(USE_SDL1)
	if ((e.type >= SDL_KEYDOWN && e.type < SDL_JOYAXISMOTION) || (e.type >= SDL_FINGERDOWN && e.type < SDL_DOLLARGESTURE)) {
#else
	if (e.type >= SDL_KEYDOWN && e.type < SDL_JOYAXISMOTION) {
#endif
		// Keyboard or Mouse (or Touch) events -> switch to standard input
		sgbControllerActive = false;
	}
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
			if (!(SDL_GetModState() & KMOD_ALT)) {
				return MenuAction_SELECT;
			}
			break;
		}
		case SDLK_KP_ENTER:
			return MenuAction_SELECT;
		case SDLK_SPACE:
			return MenuAction_SELECT;
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
#endif // !HAS_KBCTRL

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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y > 0) {
			return MenuAction_UP;
		} else if (event.wheel.y < 0) {
			return MenuAction_DOWN;
		}
	}
#else
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		switch (event.button.button) {
		case SDL_BUTTON_WHEELUP:
			return MenuAction_UP;
		case SDL_BUTTON_WHEELDOWN:
			return MenuAction_DOWN;
		}
	}
#endif

	return MenuAction_NONE;
}

DEVILUTION_END_NAMESPACE
