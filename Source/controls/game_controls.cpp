#include "game_controls.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
//#include <cstdint>

#include "../miniwin/miniwin.h"
#include "controller_motion.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

static bool startDownReceived = false;
static bool selectDownReceived = false;
bool start_modifier_active = false;
bool select_modifier_active = false;

/** Gamepad dpad acts as hotkeys without holding "start" */
bool dpad_hotkeys = false;
/** Shoulder gamepad buttons act as potions by default */
bool switch_potions_and_clicks = false;

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

	if (!inGameMenu) {
		switch (ctrlEvent.button) {
		case ControllerButton_BUTTON_LEFTSHOULDER:
			if (select_modifier_active != switch_potions_and_clicks) {
				if (!IsAutomapActive())
					*action = GameActionSendMouseClick { GameActionSendMouseClick::LEFT, ctrlEvent.up };
				return true;
			}
			break;
		case ControllerButton_BUTTON_RIGHTSHOULDER:
			if (select_modifier_active != switch_potions_and_clicks) {
				if (!IsAutomapActive())
					*action = GameActionSendMouseClick { GameActionSendMouseClick::RIGHT, ctrlEvent.up };
				return true;
			}
			break;
		case ControllerButton_AXIS_TRIGGERLEFT: // ZL (aka L2)
			if (select_modifier_active)
				*action = GameActionSendKey { DVL_VK_U, ctrlEvent.up }; // ACT_QUESTS
			else
				*action = GameActionSendKey { DVL_VK_C, ctrlEvent.up }; // ACT_CHAR
			return true;
		case ControllerButton_AXIS_TRIGGERRIGHT: // ZR (aka R2)
			if (select_modifier_active)
				*action = GameActionSendKey { DVL_VK_B, ctrlEvent.up }; // ACT_SKLBOOK
			else
				*action = GameActionSendKey { DVL_VK_I, ctrlEvent.up }; // ACT_INV
			return true;
		case ControllerButton_IGNORE:
		case ControllerButton_BUTTON_START:
		case ControllerButton_BUTTON_BACK:
			return true;
		default:
			break;
		}
		if (dpad_hotkeys) {
			switch (ctrlEvent.button) {
			case ControllerButton_BUTTON_DPAD_UP:
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_W, ctrlEvent.up }; // ACT_SKL5
				else
					*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
				return true;
			case ControllerButton_BUTTON_DPAD_RIGHT:
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_R, ctrlEvent.up }; // ACT_SKL7
				else
					*action = GameActionSendKey { DVL_VK_I, ctrlEvent.up }; // ACT_INV
				return true;
			case ControllerButton_BUTTON_DPAD_DOWN:
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_E, ctrlEvent.up }; // ACT_SKL6
				else
					*action = GameActionSendKey { DVL_VK_TAB, ctrlEvent.up }; // ACT_AUTOMAP
				return true;
			case ControllerButton_BUTTON_DPAD_LEFT:
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_Q, ctrlEvent.up }; // ACT_SKL4
				else
					*action = GameActionSendKey { DVL_VK_C, ctrlEvent.up }; // ACT_CHAR
				return true;
			default:
				break;
			}
		}
		if (start_modifier_active) {
			switch (ctrlEvent.button) {
			case ControllerButton_BUTTON_DPAD_UP:
				*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
				break;
			case ControllerButton_BUTTON_DPAD_RIGHT:
				*action = GameActionSendKey { DVL_VK_I, ctrlEvent.up }; // ACT_INV
				break;
			case ControllerButton_BUTTON_DPAD_DOWN:
				*action = GameActionSendKey { DVL_VK_TAB, ctrlEvent.up }; // ACT_AUTOMAP
				break;
			case ControllerButton_BUTTON_DPAD_LEFT:
				*action = GameActionSendKey { DVL_VK_C, ctrlEvent.up }; // ACT_CHAR
				break;
			case ControllerButton_BUTTON_FACE_TOP:
#ifdef __3DS__
				*action = GameActionSendKey { DVL_VK_Z, ctrlEvent.up }; // ACT_ZOOM
				return true;
#else
				// Not mapped. Reserved for future use.
				break;
#endif
			case ControllerButton_BUTTON_FACE_RIGHT:
				// Not mapped. TODO: map to attack in place.
				break;
			case ControllerButton_BUTTON_FACE_BOTTOM:
				*action = GameActionSendKey { DVL_VK_B, ctrlEvent.up }; // ACT_SKLBOOK
				break;
			case ControllerButton_BUTTON_FACE_LEFT:
				*action = GameActionSendKey { DVL_VK_U, ctrlEvent.up }; // ACT_QUESTS
				break;
			case ControllerButton_BUTTON_LEFTSHOULDER:
				*action = GameActionSendKey { DVL_VK_I, ctrlEvent.up }; // ACT_INV
				break;
			case ControllerButton_BUTTON_RIGHTSHOULDER:
				*action = GameActionSendKey { DVL_VK_M, ctrlEvent.up }; // ACT_TEAM
				break;
			default:
				break;
			}
			return true;
		}

		switch (ctrlEvent.button) {
		case ControllerButton_BUTTON_FACE_BOTTOM:
			// Activate second quick spell or close menus or opens quick spell book if nothing is open.
			if (!ctrlEvent.up) {
				Uint32 vk_code = DVL_VK_L; // ACT_SKLLIST
				if (select_modifier_active)
					vk_code = DVL_VK_E; // ACT_SKL6
				//else if (gbDoomflag)
				//	vk_code = DVL_VK_ESCAPE;
				else if (gnNumActiveWindows != 0) {
					switch (gaActiveWindows[gnNumActiveWindows - 1]) {
						case WND_INV:	vk_code = DVL_VK_I;	break; // ACT_INV
						case WND_CHAR:	vk_code = DVL_VK_C;	break; // ACT_CHAR
						case WND_BOOK:	vk_code = DVL_VK_B;	break; // ACT_SKLBOOK
						case WND_TEAM:	vk_code = DVL_VK_M;	break; // ACT_TEAM
						case WND_QUEST:	vk_code = DVL_VK_U;	break; // ACT_QUESTS
						default: ASSUME_UNREACHABLE;		break;
					}
				}
				*action = GameActionSendKey { vk_code, ctrlEvent.up };
			}
			return true;
		case ControllerButton_BUTTON_FACE_RIGHT:
			if (!ctrlEvent.up) {
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_R, ctrlEvent.up }; // ACT_SKL7
				else
					*action = GameActionSendKey { DVL_VK_LBUTTON, ctrlEvent.up }; // ACT_ACT
			}
			return true;
		case ControllerButton_BUTTON_FACE_TOP:
			if (!ctrlEvent.up) {
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_W, ctrlEvent.up }; // ACT_SKL5
				else
					*action = GameActionSendKey { DVL_VK_CONTROLLER_1, ctrlEvent.up }; // ACT_CTRL_ALTACT
			}
			return true;
		case ControllerButton_BUTTON_FACE_LEFT:
			if (!ctrlEvent.up) {
				if (select_modifier_active)
					*action = GameActionSendKey { DVL_VK_Q, ctrlEvent.up }; // ACT_SKL4
				else
					*action = GameActionSendKey { DVL_VK_CONTROLLER_2, ctrlEvent.up }; // ACT_CTRL_CASTACT
			}
			return true;
		case ControllerButton_BUTTON_LEFTSHOULDER:
			if (!ctrlEvent.up)
				*action = GameActionSendKey { DVL_VK_CONTROLLER_3, ctrlEvent.up }; // ACT_CTRL_USE_HP
			return true;
		case ControllerButton_BUTTON_RIGHTSHOULDER:
			if (!ctrlEvent.up)
				*action = GameActionSendKey { DVL_VK_CONTROLLER_4, ctrlEvent.up }; // ACT_CTRL_USE_MP
			return true;
		default:
			break;
		}
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

AxisDirection GetMoveDirection()
{
	return GetLeftStickOrDpadDirection(/*allow_dpad=*/!dpad_hotkeys);
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
