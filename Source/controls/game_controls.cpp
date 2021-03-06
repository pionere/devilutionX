#include "game_controls.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#include <cstdint>

#include "controls/controller_motion.h"
#include "controls/menu_controls.h"
#include "controls/modifier_hints.h"
#include "controls/plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

bool start_modifier_active = false;
bool select_modifier_active = false;

/** Gamepad dpad acts as hotkeys without holding "start" */
bool dpad_hotkeys = false;
/** Shoulder gamepad buttons act as potions by default */
bool switch_potions_and_clicks = false;

namespace {

DWORD TranslateControllerButtonToKey(ControllerButton controllerButton)
{
	switch (controllerButton) {
	case ControllerButton_BUTTON_A: // Bottom button
		return gbQuestlog ? DVL_VK_SPACE : DVL_VK_ESCAPE;
	case ControllerButton_BUTTON_B: // Right button
		return (sgpCurrentMenu != NULL || stextflag != STORE_NONE || gbQuestlog) ? DVL_VK_RETURN : DVL_VK_SPACE;
	case ControllerButton_BUTTON_Y: // Top button
		return DVL_VK_RETURN;
	case ControllerButton_BUTTON_LEFTSTICK:
		return DVL_VK_TAB; // Map
	case ControllerButton_BUTTON_BACK:
	case ControllerButton_BUTTON_START:
		return DVL_VK_ESCAPE;
	case ControllerButton_BUTTON_DPAD_LEFT:
		return DVL_VK_LEFT;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		return DVL_VK_RIGHT;
	case ControllerButton_BUTTON_DPAD_UP:
		return DVL_VK_UP;
	case ControllerButton_BUTTON_DPAD_DOWN:
		return DVL_VK_DOWN;
	default:
		return 0;
	}
}

bool HandleStartAndSelect(const ControllerButtonEvent &ctrlEvent, GameAction *action)
{
	const bool inGameMenu = InGameMenu();

	const bool startIsDown = IsControllerButtonPressed(ControllerButton_BUTTON_START);
	const bool selectIsDown = IsControllerButtonPressed(ControllerButton_BUTTON_BACK);
	start_modifier_active = !inGameMenu && startIsDown;
	select_modifier_active = !inGameMenu && selectIsDown && !start_modifier_active;

	// Tracks whether we've received both START and SELECT down events.
	//
	// Using `IsControllerButtonPressed()` for this would be incorrect.
	// If both buttons are pressed simultaneously, SDL sends 2 events for which both buttons are in the pressed state.
	// This allows us to avoid triggering START+SELECT action twice in this case.
	static bool startDownReceived = false;
	static bool selectDownReceived = false;
	switch (ctrlEvent.button) {
	case ControllerButton_BUTTON_BACK:
		selectDownReceived = !ctrlEvent.up;
		break;
	case ControllerButton_BUTTON_START:
		startDownReceived = !ctrlEvent.up;
		break;
	default:
		return false;
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

	return false;
}

} // namespace

bool GetGameAction(const SDL_Event &event, ControllerButtonEvent ctrlEvent, GameAction *action)
{
	const bool inGameMenu = InGameMenu();

	if (HandleStartAndSelect(ctrlEvent, action))
		return true;

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
			if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
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
			if (!ctrlEvent.up) {
				if (select_modifier_active)
					*action = GameAction(GameActionType_TOGGLE_QUEST_LOG);
				else
					*action = GameAction(GameActionType_TOGGLE_CHARACTER_INFO);
			}
			return true;
		case ControllerButton_AXIS_TRIGGERRIGHT: // ZR (aka R2)
			if (!ctrlEvent.up) {
				if (select_modifier_active)
					*action = GameAction(GameActionType_TOGGLE_SPELL_BOOK);
				else
					*action = GameAction(GameActionType_TOGGLE_INVENTORY);
			}
			return true;
		case ControllerButton_IGNORE:
		case ControllerButton_BUTTON_START:
		case ControllerButton_BUTTON_BACK:
			return true;
			break;
		default:
			break;
		}
		if (dpad_hotkeys) {
			switch (ctrlEvent.button) {
			case ControllerButton_BUTTON_DPAD_UP:
				if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
					*action = GameActionSendKey { DVL_VK_F6, ctrlEvent.up };
				else
					*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
				return true;
			case ControllerButton_BUTTON_DPAD_RIGHT:
				if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
					*action = GameActionSendKey { DVL_VK_F8, ctrlEvent.up };
				else if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_INVENTORY);
				return true;
			case ControllerButton_BUTTON_DPAD_DOWN:
				if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
					*action = GameActionSendKey { DVL_VK_F7, ctrlEvent.up };
				else
					*action = GameActionSendKey { DVL_VK_TAB, ctrlEvent.up };
				return true;
			case ControllerButton_BUTTON_DPAD_LEFT:
				if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
					*action = GameActionSendKey { DVL_VK_F5, ctrlEvent.up };
				else if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_CHARACTER_INFO);
				return true;
			default:
				break;
			}
		}
		if (start_modifier_active) {
			switch (ctrlEvent.button) {
			case ControllerButton_BUTTON_DPAD_UP:
				*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
				return true;
			case ControllerButton_BUTTON_DPAD_RIGHT:
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_INVENTORY);
				return true;
			case ControllerButton_BUTTON_DPAD_DOWN:
				*action = GameActionSendKey { DVL_VK_TAB, ctrlEvent.up };
				return true;
			case ControllerButton_BUTTON_DPAD_LEFT:
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_CHARACTER_INFO);
				return true;
			case ControllerButton_BUTTON_Y: // Top button
#ifdef __3DS__
				if (!ctrlEvent.up) {
					gbZoomflag = !gbZoomflag;
					CalcViewportGeometry();
				}
#else
				// Not mapped. Reserved for future use.
#endif
				return true;
			case ControllerButton_BUTTON_B: // Right button
				// Not mapped. TODO: map to attack in place.
				return true;
			case ControllerButton_BUTTON_A: // Bottom button
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_SPELL_BOOK);
				return true;
			case ControllerButton_BUTTON_X: // Left button
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_QUEST_LOG);
				return true;
			case ControllerButton_BUTTON_LEFTSHOULDER:
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_CHARACTER_INFO);
				return true;
			case ControllerButton_BUTTON_RIGHTSHOULDER:
				if (!ctrlEvent.up)
					*action = GameAction(GameActionType_TOGGLE_INVENTORY);
				return true;
			default:
				return true;
			}
		}

		// Bottom button: Closes menus or opens quick spell book if nothing is open.
		if (ctrlEvent.button == ControllerButton_BUTTON_A) { // Bottom button
			if (ctrlEvent.up)
				return true;
			if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
				*action = GameActionSendKey { DVL_VK_F7, ctrlEvent.up };
			else if (gbDoomflag)
				*action = GameActionSendKey { DVL_VK_ESCAPE, ctrlEvent.up };
			else if (gbInvflag)
				*action = GameAction(GameActionType_TOGGLE_INVENTORY);
			else if (gbSbookflag)
				*action = GameAction(GameActionType_TOGGLE_SPELL_BOOK);
			else if (gbQuestlog)
				*action = GameAction(GameActionType_TOGGLE_QUEST_LOG);
			else if (gbChrflag)
				*action = GameAction(GameActionType_TOGGLE_CHARACTER_INFO);
			else
				*action = GameAction(GameActionType_TOGGLE_QUICK_SPELL_MENU);
			return true;
		}

		if (!gbQuestlog && !gbSbookflag) {
			switch (ctrlEvent.button) {
			case ControllerButton_IGNORE:
				return true;
			case ControllerButton_BUTTON_B: // Right button
				if (!ctrlEvent.up) {
					if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
						*action = GameActionSendKey { DVL_VK_F8, ctrlEvent.up };
					else
						*action = GameAction(GameActionType_PRIMARY_ACTION);
				}
				return true;
			case ControllerButton_BUTTON_Y: // Top button
				if (!ctrlEvent.up) {
					if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
						*action = GameActionSendKey { DVL_VK_F6, ctrlEvent.up };
					else
						*action = GameAction(GameActionType_SECONDARY_ACTION);
				}
				return true;
			case ControllerButton_BUTTON_X: // Left button
				if (!ctrlEvent.up) {
					if (IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
						*action = GameActionSendKey { DVL_VK_F5, ctrlEvent.up };
					else
						*action = GameAction(GameActionType_CAST_SPELL);
				}
				return true;
			case ControllerButton_BUTTON_LEFTSHOULDER:
				if (stextflag == STORE_NONE && !ctrlEvent.up)
					*action = GameAction(GameActionType_USE_HEALTH_POTION);
				return true;
			case ControllerButton_BUTTON_RIGHTSHOULDER:
				if (stextflag == STORE_NONE && !ctrlEvent.up)
					*action = GameAction(GameActionType_USE_MANA_POTION);
				return true;
			case ControllerButton_BUTTON_DPAD_UP:
			case ControllerButton_BUTTON_DPAD_DOWN:
			case ControllerButton_BUTTON_DPAD_LEFT:
			case ControllerButton_BUTTON_DPAD_RIGHT:
				// The rest of D-Pad actions are handled in charMovement() on every game_logic() call.
				return true;
			default:
				break;
			}
		}

		if (ctrlEvent.button == ControllerButton_BUTTON_BACK) {
			return true; // Ignore mod button
		}
	}

	// DPad navigation is handled separately for these.
	if (gmenu_is_active() || gbQuestlog || stextflag != STORE_NONE) {
		switch (ctrlEvent.button) {
		case ControllerButton_BUTTON_DPAD_UP:
		case ControllerButton_BUTTON_DPAD_DOWN:
		case ControllerButton_BUTTON_DPAD_LEFT:
		case ControllerButton_BUTTON_DPAD_RIGHT:
			return true;
		default:
			break;
		}
	}

	// By default, map to a keyboard key.
	if (ctrlEvent.button != ControllerButton_NONE) {
		*action = GameActionSendKey { TranslateControllerButtonToKey(ctrlEvent.button),
			ctrlEvent.up };
		return true;
	}

#ifndef USE_SDL1
 #if HAS_JOYSTICK == 1 && HAS_GAMECTRL == 1
	// Ignore unhandled joystick events where a GameController is open for this joystick.
	// This is because SDL sends both game controller and joystick events in this case.
	const Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL && GameController::Get(joystick->instance_id()) != NULL) {
		return true;
	}
#endif
	if (event.type == SDL_CONTROLLERAXISMOTION) {
		return true; // Ignore releasing the trigger buttons
	}
#endif

	return false;
}

AxisDirection GetMoveDirection()
{
	return GetLeftStickOrDpadDirection(/*allow_dpad=*/!dpad_hotkeys);
}

DEVILUTION_END_NAMESPACE

#endif
