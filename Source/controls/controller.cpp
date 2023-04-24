#include "controller.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

DEVILUTION_BEGIN_NAMESPACE

ControllerButtonEvent ToControllerButtonEvent(const SDL_Event& event)
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

#if HAS_KBCTRL
	result.button = KbCtrlToControllerButton(event);
	if (result.button != ControllerButton_NONE)
		return result;
#endif

#if HAS_GAMECTRL
	result.button = GameController::ToControllerButton(event);
	if (result.button != ControllerButton_NONE)
		return result;
#endif

#if HAS_JOYSTICK
	result.button = Joystick::ToControllerButton(event);
	if (result.button != ControllerButton_NONE)
		return result;
#endif
	return result;
}

bool IsControllerButtonPressed(ControllerButton button)
{
#if HAS_GAMECTRL
	if (GameController::IsPressedOnAnyController(button))
		return true;
#endif
#if HAS_KBCTRL
	if (IsKbCtrlButtonPressed(button))
		return true;
#endif
#if HAS_JOYSTICK
	if (Joystick::IsPressedOnAnyJoystick(button))
		return true;
#endif
	return false;
}

bool HandleControllerAddedOrRemovedEvent(const SDL_Event& event)
{
#ifndef USE_SDL1
	switch (event.type) {
#if HAS_GAMECTRL
	case SDL_CONTROLLERDEVICEADDED:
		GameController::Add(event.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		GameController::Remove(event.cdevice.which);
		break;
#endif
#if HAS_JOYSTICK
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
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
