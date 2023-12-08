#pragma once

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include "./axis_direction.h"
#include "./controller.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GameActionType {
	GameActionType_NONE = 0,
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

bool GetGameAction(const ControllerButtonEvent& ctrlEvent, GameAction* action);

extern bool start_modifier_active;
extern bool select_modifier_active;
extern bool dpad_hotkeys;

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
