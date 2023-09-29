#include "game_controller.h"

#if HAS_GAMECTRL
#include <cstddef>

#include "appfat.h"
#include "../controller.h"
#include "../controller_motion.h"
#include "utils/log.h"

DEVILUTION_BEGIN_NAMESPACE

std::vector<GameController> GameController::controllers_;

ControllerButton GameController::ToControllerButton(const SDL_Event& event)
{
	GameController* controller = GameController::Get(event);
	if (controller == NULL) {
		return ControllerButton_NONE;
	}

	switch (event.type) {
	case SDL_CONTROLLERAXISMOTION:
		switch (event.caxis.axis) {
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			if (event.caxis.value < 8192) { // 25% pressed
				controller->trigger_left_is_down_ = false;
			}
			if (event.caxis.value > 16384 && !controller->trigger_left_is_down_) { // 50% pressed
				controller->trigger_left_is_down_ = true;
				return ControllerButton_AXIS_TRIGGERLEFT;
			}
			return ControllerButton_NONE;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			if (event.caxis.value < 8192) { // 25% pressed
				controller->trigger_right_is_down_ = false;
			}
			if (event.caxis.value > 16384 && !controller->trigger_right_is_down_) { // 50% pressed
				controller->trigger_right_is_down_ = true;
				return ControllerButton_AXIS_TRIGGERRIGHT;
			}
			return ControllerButton_NONE;
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		switch (event.cbutton.button) {
		case SDL_CONTROLLER_BUTTON_A:
			return ControllerButton_BUTTON_FACE_BOTTOM;
		case SDL_CONTROLLER_BUTTON_B:
			return ControllerButton_BUTTON_FACE_RIGHT;
		case SDL_CONTROLLER_BUTTON_X:
			return ControllerButton_BUTTON_FACE_LEFT;
		case SDL_CONTROLLER_BUTTON_Y:
			return ControllerButton_BUTTON_FACE_TOP;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return ControllerButton_BUTTON_LEFTSTICK;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return ControllerButton_BUTTON_RIGHTSTICK;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			return ControllerButton_BUTTON_LEFTSHOULDER;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			return ControllerButton_BUTTON_RIGHTSHOULDER;
		case SDL_CONTROLLER_BUTTON_START:
			return ControllerButton_BUTTON_START;
		case SDL_CONTROLLER_BUTTON_BACK:
			return ControllerButton_BUTTON_BACK;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return ControllerButton_BUTTON_DPAD_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return ControllerButton_BUTTON_DPAD_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return ControllerButton_BUTTON_DPAD_LEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return ControllerButton_BUTTON_DPAD_RIGHT;
		default:
			break;
		}
	default:
		break;
	}
	return ControllerButton_NONE;
}

SDL_GameControllerButton GameController::ToSdlGameControllerButton(ControllerButton button)
{
	switch (button) {
	case ControllerButton_NONE:
	case ControllerButton_IGNORE:
		break;
	case ControllerButton_BUTTON_FACE_BOTTOM:
		return SDL_CONTROLLER_BUTTON_A;
	case ControllerButton_BUTTON_FACE_RIGHT:
		return SDL_CONTROLLER_BUTTON_B;
	case ControllerButton_BUTTON_FACE_LEFT:
		return SDL_CONTROLLER_BUTTON_X;
	case ControllerButton_BUTTON_FACE_TOP:
		return SDL_CONTROLLER_BUTTON_Y;
	case ControllerButton_BUTTON_BACK:
		return SDL_CONTROLLER_BUTTON_BACK;
	case ControllerButton_BUTTON_START:
		return SDL_CONTROLLER_BUTTON_START;
	case ControllerButton_BUTTON_LEFTSTICK:
		return SDL_CONTROLLER_BUTTON_LEFTSTICK;
	case ControllerButton_BUTTON_RIGHTSTICK:
		return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
	case ControllerButton_BUTTON_LEFTSHOULDER:
		return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	case ControllerButton_BUTTON_RIGHTSHOULDER:
		return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
	case ControllerButton_BUTTON_DPAD_UP:
		return SDL_CONTROLLER_BUTTON_DPAD_UP;
	case ControllerButton_BUTTON_DPAD_DOWN:
		return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
	case ControllerButton_BUTTON_DPAD_LEFT:
		return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
	case ControllerButton_AXIS_TRIGGERLEFT:
	case ControllerButton_AXIS_TRIGGERRIGHT:
		ASSUME_UNREACHABLE
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return SDL_CONTROLLER_BUTTON_INVALID;
}

bool GameController::IsPressed(ControllerButton button) const
{
	const SDL_GameControllerButton gcButton = ToSdlGameControllerButton(button);
	return gcButton != SDL_CONTROLLER_BUTTON_INVALID && SDL_GameControllerGetButton(sdl_game_controller_, gcButton) != 0;
}

bool GameController::ProcessAxisMotion(const SDL_Event& event)
{
	if (event.type != SDL_CONTROLLERAXISMOTION)
		return false;
	if (Get(event.caxis.which) == NULL)
		return false;
	switch (event.caxis.axis) {
	case SDL_CONTROLLER_AXIS_LEFTX:
		leftStickXUnscaled = event.caxis.value;
		ScaleJoystickAxes(false);
		break;
	case SDL_CONTROLLER_AXIS_LEFTY:
		leftStickYUnscaled = -event.caxis.value;
		ScaleJoystickAxes(false);
		break;
	case SDL_CONTROLLER_AXIS_RIGHTX:
		rightStickXUnscaled = event.caxis.value;
		ScaleJoystickAxes(true);
		break;
	case SDL_CONTROLLER_AXIS_RIGHTY:
		rightStickYUnscaled = -event.caxis.value;
		ScaleJoystickAxes(true);
		break;
	default:
		return false;
	}
	return true;
}

void GameController::Add(int joystickIndex)
{
	DoLog("Opening game controller for joystick at index %d", joystickIndex);
	GameController result;
	result.sdl_game_controller_ = SDL_GameControllerOpen(joystickIndex);
	if (result.sdl_game_controller_ == NULL) {
		DoLog(SDL_GetError());
		return;
	}
	SDL_Joystick* sdlJoystick = SDL_GameControllerGetJoystick(result.sdl_game_controller_);
	result.instance_id_ = SDL_JoystickInstanceID(sdlJoystick);
	controllers_.push_back(result);
	sgbControllerActive = true;

	SDL_JoystickGUID guid = SDL_JoystickGetGUID(sdlJoystick);
	char* mapping = SDL_GameControllerMappingForGUID(guid);
	DoLog("Opened game controller with mapping:\n%s", mapping);
	SDL_free(mapping);
}

void GameController::Remove(SDL_JoystickID instanceId)
{
	DoLog("Removing game controller with instance id %d", instanceId);
	for (unsigned i = 0; i < controllers_.size(); ++i) {
		const GameController& controller = controllers_[i];
		if (controller.instance_id_ != instanceId)
			continue;
		SDL_GameControllerClose(controller.sdl_game_controller_);
		controllers_.erase(controllers_.begin() + i);
		sgbControllerActive = !controllers_.empty();
		return;
	}
	DoLog("Game controller not found with instance id: %d", instanceId);
}

GameController* GameController::Get(SDL_JoystickID instanceId)
{
	for (unsigned i = 0; i < controllers_.size(); ++i) {
		GameController& controller = controllers_[i];
		if (controller.instance_id_ == instanceId)
			return &controller;
	}
	return NULL;
}

GameController* GameController::Get(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_CONTROLLERAXISMOTION:
		return Get(event.caxis.which);
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		return Get(event.jball.which);
	default:
		return NULL;
	}
}

void GameController::ReleaseAll()
{
	while (!controllers_.empty()) {
		GameController::Remove(controllers_.front().instance_id_);
	}
}

bool GameController::IsPressedOnAnyController(ControllerButton button)
{
	for (unsigned i = 0; i < controllers_.size(); ++i)
		if (controllers_[i].IsPressed(button))
			return true;
	return false;
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL
