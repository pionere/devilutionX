#pragma once

#if HAS_JOYSTICK
// Joystick mappings for SDL1 and additional buttons on SDL2.

#include <vector>

#include <SDL.h>
#include "utils/sdl2_to_1_2_backports.h"

#include "../controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

class Joystick {
	static std::vector<Joystick> joysticks_;

public:
	static void Add(int device_index);
#ifndef USE_SDL1
	static void Remove(SDL_JoystickID instance_id);
#endif
	// static void ReleaseAll();
	static bool IsPressedOnAnyJoystick(ControllerButton button);

	static ControllerButton ToControllerButton(const SDL_Event& event);
	static bool ProcessAxisMotion(const SDL_Event& event);

private:
	static int ToSdlJoyButton(ControllerButton button);
	static Joystick* Get(SDL_JoystickID instance_id);
	static Joystick* Get(const SDL_Event& event);

	bool IsPressed(ControllerButton button) const;
	bool IsHatButtonPressed(ControllerButton button) const;

	SDL_Joystick* sdl_joystick_ = NULL;
	SDL_JoystickID instance_id_ = -1;
};

DEVILUTION_END_NAMESPACE
#endif // HAS_JOYSTICK
