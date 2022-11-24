#pragma once

#include "../types.h"

#if HAS_GAMECTRL
#ifdef USE_SDL1
static_assert(FALSE, "GameController is not supported in SDL1.");
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
	static GameController* Get(SDL_JoystickID instance_id);
	static GameController* Get(const SDL_Event& event);
	static bool IsPressedOnAnyController(ControllerButton button);

	// NOTE: Not idempotent.
	// Must be called exactly once for each SDL input event.
	ControllerButton ToControllerButton(const SDL_Event& event);

	bool IsPressed(ControllerButton button) const;
	static bool ProcessAxisMotion(const SDL_Event& event);

private:
	static SDL_GameControllerButton ToSdlGameControllerButton(ControllerButton button);

	SDL_GameController* sdl_game_controller_ = NULL;
	SDL_JoystickID instance_id_ = -1;

	bool trigger_left_is_down_ = false;
	bool trigger_right_is_down_ = false;
};

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL
