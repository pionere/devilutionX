#include "menu_controls.h"

#include "controller.h"
#include "controller_motion.h"
#include "axis_direction.h"
//#include "game_controls.h"
#include "./devices/game_controller.h"
#include "./devices/joystick.h"

#include "DiabloUI/diabloui.h"
#include "remap_keyboard.h"

DEVILUTION_BEGIN_NAMESPACE

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
MenuAction GetMenuHeldUpDownAction()
{
	const AxisDirection dir = axisDirRepeater.Get(GetLeftStickOrDpadDirection(true));
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

MenuAction GetMenuAction(const SDL_Event& event)
{
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	const ControllerButtonEvent ctrlEvent = ToControllerButtonEvent(event);

	if (ProcessControllerMotion(event)) {
		sgbControllerActive = true;
		return MenuAction_NONE;
	}
#if HAS_DPAD
	if (/*!dpad_hotkeys &&*/ SimulateRightStickWithDpad(ctrlEvent)) {
		sgbControllerActive = true;
		return MenuAction_NONE;
	}
#endif

	if (ctrlEvent.button != ControllerButton_NONE)
		sgbControllerActive = true;

	if (!ctrlEvent.up) {
		switch (ctrlEvent.button) {
		case ControllerButton_IGNORE:
			return MenuAction_NONE;
		case ControllerButton_BUTTON_FACE_RIGHT:
		case ControllerButton_BUTTON_START:
			return MenuAction_SELECT;
		case ControllerButton_BUTTON_BACK:
		case ControllerButton_BUTTON_FACE_BOTTOM:
			return MenuAction_BACK;
		case ControllerButton_BUTTON_FACE_LEFT:
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
			return (event.key.keysym.mod & KMOD_SHIFT) ? MenuAction_UP : MenuAction_DOWN;
		case SDLK_PAGEUP:
			return MenuAction_PAGE_UP;
		case SDLK_PAGEDOWN:
			return MenuAction_PAGE_DOWN;
		case SDLK_RETURN: {
			if (!(event.key.keysym.mod & KMOD_ALT)) {
				return MenuAction_SELECT;
			}
			break;
		}
		case SDLK_KP_ENTER:
			return MenuAction_SELECT;
		case SDLK_SPACE:
			return MenuAction_SELECT;
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

#ifndef USE_SDL1
#if HAS_GAMECTRL
	if (event.type == SDL_CONTROLLERDEVICEADDED) {
		GameController::Add(event.cdevice.which);
	}
	if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
		GameController::Remove(event.cdevice.which);
	}
#endif
#if HAS_JOYSTICK
	if (event.type == SDL_JOYDEVICEADDED) {
		Joystick::Add(event.jdevice.which);
	}
	if (event.type == SDL_JOYDEVICEREMOVED) {
		Joystick::Remove(event.jdevice.which);
	}
#endif
#endif // USE_SDL1
	return MenuAction_NONE;
}

DEVILUTION_END_NAMESPACE
