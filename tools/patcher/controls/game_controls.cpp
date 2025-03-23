#include "game_controls.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
//#include <cstdint>

#include "../miniwin/miniwin.h"
#include "controller_motion.h"
//#include "plrctrls.h"
#define InGameMenu() true
//#include "../automap.h"
#define IsAutomapActive() false
//#include "../control.h"
#define gnNumActiveWindows 0

DEVILUTION_BEGIN_NAMESPACE

static bool startDownReceived = false;
static bool selectDownReceived = false;

bool start_modifier_active = false;
bool select_modifier_active = false;

/** Gamepad dpad acts as hotkeys without holding "start" */
bool dpad_hotkeys = false;

static uint32_t TranslateControllerButtonToKey(ControllerButton controllerButton)
{
	switch (controllerButton) {
	case ControllerButton_BUTTON_FACE_BOTTOM:
		return DVL_VK_ESCAPE;
	case ControllerButton_BUTTON_FACE_RIGHT:
		return DVL_VK_RETURN;
	case ControllerButton_BUTTON_FACE_TOP:
		return DVL_VK_RETURN;
	case ControllerButton_BUTTON_LEFTSTICK:
		return DVL_VK_TAB; // Map
	case ControllerButton_BUTTON_BACK:
	case ControllerButton_BUTTON_START:
		return DVL_VK_ESCAPE;
	/*case ControllerButton_BUTTON_DPAD_LEFT:
		return DVL_VK_LEFT;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		return DVL_VK_RIGHT;
	case ControllerButton_BUTTON_DPAD_UP:
		return DVL_VK_UP;
	case ControllerButton_BUTTON_DPAD_DOWN:
		return DVL_VK_DOWN;*/
	default:
		return 0;
	}
}

bool GetGameAction(const ControllerButtonEvent& ctrlEvent, GameAction* action)
{
	const bool inGameMenu = InGameMenu();
	const bool startIsDown = IsControllerButtonPressed(ControllerButton_BUTTON_START);
	const bool selectIsDown = IsControllerButtonPressed(ControllerButton_BUTTON_BACK);

	start_modifier_active = !inGameMenu && startIsDown;
	select_modifier_active = !inGameMenu && selectIsDown && !start_modifier_active;

	// Handle start and select
	// Tracks whether we've received both START and SELECT down events.
	//
	// Using `IsControllerButtonPressed()` for this would be incorrect.
	// If both buttons are pressed simultaneously, SDL sends 2 events for which both buttons are in the pressed state.
	// This allows us to avoid triggering START+SELECT action twice in this case.
	switch (ctrlEvent.button) {
	case ControllerButton_BUTTON_BACK:
	case ControllerButton_BUTTON_START:
		if (ctrlEvent.button == ControllerButton_BUTTON_BACK) {
			selectDownReceived = !ctrlEvent.up;
		} else {
			startDownReceived = !ctrlEvent.up;
		}
		if (startDownReceived && selectDownReceived) {
			*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
			return true;
		}

		if (inGameMenu && (startIsDown || selectIsDown) && !ctrlEvent.up) {
			// If both are down, do nothing because `both_received` will trigger soon.
			if (startIsDown && selectIsDown)
				return true;
			*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
			return true;
		}
		break;
	default:
		break;
	}

	// Stick clicks simulate the mouse both in menus and in-game.
	switch (ctrlEvent.button) {
	case ControllerButton_BUTTON_LEFTSTICK:
		if (select_modifier_active) {
			if (!IsAutomapActive())
				*action = GameActionSendMouseClick { GameActionSendMouseClick::LEFT, ctrlEvent.up };
			return true;
		}
		break;
	case ControllerButton_BUTTON_RIGHTSTICK:
		if (!IsAutomapActive()) {
			if (select_modifier_active)
				*action = GameActionSendMouseClick { GameActionSendMouseClick::RIGHT, ctrlEvent.up };
			else
				*action = GameActionSendMouseClick { GameActionSendMouseClick::LEFT, ctrlEvent.up };
		}
		return true;
	default:
		break;
	}

	// DPad navigation is handled separately.
	// - movement in store and other windows are handled in plrctrls_every_frame
	// - standard movement is handled in plrctrls_after_game_logic
	switch (ctrlEvent.button) {
	case ControllerButton_BUTTON_DPAD_UP:
	case ControllerButton_BUTTON_DPAD_DOWN:
	case ControllerButton_BUTTON_DPAD_LEFT:
	case ControllerButton_BUTTON_DPAD_RIGHT:
		return true;
	default:
		break;
	}

	// By default, map to a keyboard key.
	if (ctrlEvent.button != ControllerButton_NONE) {
		*action = GameActionSendKey { TranslateControllerButtonToKey(ctrlEvent.button),
			ctrlEvent.up };
		return true;
	}

	return false;
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
