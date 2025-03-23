#pragma once

#if HAS_GAMECTRL
#ifdef USE_SDL1
static_assert(false, "GameController is not supported in SDL1.");
#endif

#include <vector>

#include <SDL.h>

#include "../controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

class GameController {
	static std::vector<GameController> controllers_;

public:
	static void Add(int joystick_index);
	static void Remove(SDL_JoystickID instance_id);
	static void ReleaseAll();
	static GameController* Get(const SDL_Event& event);
	static GameController* Get(SDL_JoystickID instance_id);
	static bool IsPressedOnAnyController(ControllerButton button);

	// NOTE: Not idempotent.
	// Must be called exactly once for each SDL input event.
	static ControllerButton ToControllerButton(const SDL_Event& event);
	static bool ProcessAxisMotion(const SDL_Event& event);

private:
	static SDL_GameControllerButton ToSdlGameControllerButton(ControllerButton button);

	bool IsPressed(ControllerButton button) const;

	SDL_GameController* sdl_game_controller_ = NULL;
	SDL_JoystickID instance_id_ = -1;

	bool trigger_left_is_down_ = false;
	bool trigger_right_is_down_ = false;
};

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL
