#include "controller.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

DEVILUTION_BEGIN_NAMESPACE

ControllerButtonEvent ToControllerButtonEvent(const SDL_Event &event)
{
	ControllerButtonEvent result { ControllerButton_NONE, false };
	switch (event.type) {
#ifndef USE_SDL1
	case SDL_CONTROLLERBUTTONUP:
#endif
	case SDL_JOYBUTTONUP:
	case SDL_KEYUP:
		result.up = true;
		break;
	default:
		break;
	}

#if HAS_KBCTRL == 1
	result.button = KbCtrlToControllerButton(event);
	if (result.button != ControllerButton_NONE)
		return result;
#endif

#if HAS_GAMECTRL == 1
	GameController *const controller = GameController::Get(event);
	if (controller != NULL) {
		result.button = controller->ToControllerButton(event);
		if (result.button != ControllerButton_NONE)
			return result;
	}
#endif

#if HAS_JOYSTICK == 1
	const Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL)
		result.button = joystick->ToControllerButton(event);
#endif
	return result;
}

bool IsControllerButtonPressed(ControllerButton button)
{
#if HAS_GAMECTRL == 1
	if (GameController::IsPressedOnAnyController(button))
		return true;
#endif
#if HAS_KBCTRL == 1
	if (IsKbCtrlButtonPressed(button))
		return true;
#endif
#if HAS_JOYSTICK == 1
	if (Joystick::IsPressedOnAnyJoystick(button))
		return true;
#endif
	return false;
}

bool HandleControllerAddedOrRemovedEvent(const SDL_Event &event)
{
#ifndef USE_SDL1
	switch (event.type) {
#if HAS_GAMECTRL == 1
	case SDL_CONTROLLERDEVICEADDED:
		GameController::Add(event.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		GameController::Remove(event.cdevice.which);
		break;
#endif
#if HAS_JOYSTICK == 1
	case SDL_JOYDEVICEADDED:
		Joystick::Add(event.jdevice.which);
		break;
	case SDL_JOYDEVICEREMOVED:
		Joystick::Remove(event.jdevice.which);
		break;
#endif
	default:
		return false;
	}
	return true;
#else
	return false;
#endif
}

DEVILUTION_END_NAMESPACE

#endif
