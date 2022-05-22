#pragma once

#include <cstdint>
#include <SDL.h>

#include "./axis_direction.h"
#include "./controller.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GameActionType {
	GameActionType_NONE = 0,
	GameActionType_USE_HEALTH_POTION,
	GameActionType_USE_MANA_POTION,
	GameActionType_PRIMARY_ACTION,   // Talk to towners, click on inv items, attack, etc.
	GameActionType_SECONDARY_ACTION, // Open chests, doors, pickup items.
	GameActionType_CAST_SPELL,
	GameActionType_TOGGLE_INVENTORY,
	GameActionType_TOGGLE_CHARACTER_INFO,
	GameActionType_TOGGLE_SKILL_LIST,
	GameActionType_TOGGLE_SPELL_BOOK,
	GameActionType_TOGGLE_QUEST_LOG,
	GameActionType_SEND_KEY,
	GameActionType_SEND_MOUSE_CLICK,
} GameActionType;

typedef struct GameActionSendKey {
	Uint32 vk_code;
	bool up;
} GameActionSendKey;

typedef struct GameActionSendMouseClick {
	enum Button {
		LEFT = 0,
		RIGHT,
	};
	Button button;
	bool up;
} GameActionSendMouseClick;

typedef struct GameAction {
	GameActionType type;

	GameAction()
	    : type(GameActionType_NONE)
	{
	}

	explicit GameAction(GameActionType type)
	    : type(type)
	{
	}

	GameAction(GameActionSendKey send_key)
	    : type(GameActionType_SEND_KEY)
	    , send_key(send_key)
	{
	}

	GameAction(GameActionSendMouseClick send_mouse_click)
	    : type(GameActionType_SEND_MOUSE_CLICK)
	    , send_mouse_click(send_mouse_click)
	{
	}

	union {
		GameActionSendKey send_key;
		GameActionSendMouseClick send_mouse_click;
	};
} GameAction;

bool GetGameAction(const SDL_Event &event, ControllerButtonEvent ctrlEvent, GameAction *action);

AxisDirection GetMoveDirection();

extern bool start_modifier_active;
extern bool select_modifier_active;
extern bool dpad_hotkeys;
extern bool switch_potions_and_clicks;

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif
