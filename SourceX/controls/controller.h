#pragma once

#include "controls/controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

struct ControllerButtonEvent {
	ControllerButton button;
	bool up;
};

ControllerButtonEvent ToControllerButtonEvent(const SDL_Event &event);

bool IsControllerButtonPressed(ControllerButton button);

void InitController();

DEVILUTION_END_NAMESPACE
