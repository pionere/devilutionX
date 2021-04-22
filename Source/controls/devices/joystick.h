#pragma once

#include "../types.h"

#ifndef HAS_JOYSTICK
#define HAS_JOYSTICK 0
#endif

#if HAS_JOYSTICK == 1
// Joystick mappings for SDL1 and additional buttons on SDL2.

#include <vector>

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

#include "../controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

class Joystick {
	static std::vector<Joystick> *const joysticks_;

public:
	static void Add(int device_index);
	static void Remove(SDL_JoystickID instance_id);
	static Joystick *Get(SDL_JoystickID instance_id);
	static Joystick *Get(const SDL_Event &event);
	static const std::vector<Joystick> &All();
	static bool IsPressedOnAnyJoystick(ControllerButton button);

	static ControllerButton ToControllerButton(const SDL_Event &event);
	bool IsPressed(ControllerButton button) const;
	static bool ProcessAxisMotion(const SDL_Event &event);

	SDL_JoystickID instance_id() const
	{
		return instance_id_;
	}

private:
	static int ToSdlJoyButton(ControllerButton button);
	static bool IsHatButtonPressed(ControllerButton button);

	SDL_Joystick *sdl_joystick_ = NULL;
	SDL_JoystickID instance_id_ = -1;
};

DEVILUTION_END_NAMESPACE
#endif
