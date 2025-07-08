#include "touch.h"
#if HAS_TOUCHPAD
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
#include "storm/storm_cfg.h"
#include "utils/display.h"
#include "utils/sdl2_backports.h"
#include "../gameui.h"
#include "../diablo.h"
#include "../miniwin/miniwin.h"
#include <math.h>

#ifdef __vita__
static bool back_touch = false;
#endif

DEVILUTION_BEGIN_NAMESPACE

// number of handled 'direct' touch panels/screen
#define TOUCH_PORT_MAX_NUM   1
// number of simulated click-types (left or right)
#define TOUCH_PORT_CLICK_NUM 2
// finger id setting if finger is not touching the panel/screen
#define NO_TOUCH (-1)
// number of fingers to track per panel/screen
#define MAX_NUM_FINGERS 3
// taps longer than this will not result in mouse click events (ms)
#define MAX_TAP_TIME 250
// number of handled gamepad buttons
#define MAX_GAMEPAD_BUTTONS 16
// number of handled fingers on a gamepad button
#define MAX_GAMEPAD_FINGERS 2

typedef struct FingerStruct {
	SDL_FingerID id; // -1: not touching
	Uint32 time_last_down;
	int last_x;      // last known screen x-coordinate
	int last_y;      // last known screen y-coordinate
	int first_x;     // first screen x-coordinate
	int first_y;     // first screen y-coordinate
} FingerStruct;

static FingerStruct finger[TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS]; // keep track of finger status

enum DraggingType {
	DRAG_NONE,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
	DRAG_OVER,
};

static DraggingType multi_finger_dragging[TOUCH_PORT_MAX_NUM]; // keep track whether we are currently drag-and-dropping

/** virtual gamepad icons  CEL */
static CelImageBuf* pGamepadIconsCels;

typedef struct ButtonStruct {
	int buSize;
	int buPosX;
	int buPosY;
	int buVkCode;
	int buFrame;
	char buLabel;

	bool buPressed;
	SDL_FingerID buFinger[MAX_GAMEPAD_FINGERS];
} ButtonStruct;

static ButtonStruct gaButtons[2][MAX_GAMEPAD_BUTTONS];

#define SetMouseButtonEvent(/*SDL_Event* */evt, /*uint32_t */ev_type, /*uint8_t */btn, /*uint8_t */btn_state, /*int32_t */bx, /*int32_t */by) \
{                                    \
	(evt)->type = ev_type;           \
	(evt)->button.button = btn;      \
	(evt)->button.state = btn_state; \
	(evt)->button.x = bx;            \
	(evt)->button.y = by;            \
}

#define SetMouseMotionEvent(/*SDL_Event* */evt, /*int32_t */mx, /*int32_t */my) \
{                                  \
	(evt)->type = SDL_MOUSEMOTION; \
	(evt)->motion.x = mx;          \
	(evt)->motion.y = my;          \
}

static void init_virtual_gamepad()
{
	// load default gamepad
#if VIRTUAL_GAMEPAD
	gaButtons[0][0] = { 67,   0, 137, DVL_VK_LEFT,  1, 'L', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */ };
	gaButtons[0][1] = { 67, 137, 137, DVL_VK_RIGHT, 1, 'R', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[0][2] = { 67,  70,  67, DVL_VK_UP,    1, 'U', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[0][3] = { 67,  70, 207, DVL_VK_DOWN,  1, 'D', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};

	gaButtons[0][4] = { 67, 893,  67, DVL_VK_RETURN, 10, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[0][5] = { 67, 893, 207, DVL_VK_ESCAPE,  9, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};

	gaButtons[1][0] = { 67,   0,  67, DVL_VK_A, 1, 'A', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][1] = { 67,  70,  67, DVL_VK_S, 1, 'S', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][2] = { 67,   0, 137, DVL_VK_D, 1, 'D', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][3] = { 67,  70, 137, DVL_VK_F, 1, 'F', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};

	gaButtons[1][4] = { 67, 823,  67, DVL_VK_Q, 1, 'Q', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][5] = { 67, 893,  67, DVL_VK_W, 1, 'W', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][6] = { 67, 823, 137, DVL_VK_E, 1, 'E', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][7] = { 67, 893, 137, DVL_VK_R, 1, 'R', false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};

	gaButtons[1][8] = { 67,   0, 207, DVL_VK_LSHIFT, 6, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][9] = { 67,   0, 277, DVL_VK_LMENU,  5, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};

	gaButtons[1][10] = { 67, 893, 207, DVL_VK_RSHIFT, 6, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
	gaButtons[1][11] = { 67, 893, 277, DVL_VK_ESCAPE, 9, 0, false, { 0, 0 }/*false, { NO_TOUCH, NO_TOUCH } */};
#endif
	for (int i = 0; i < 2; i++) {
		// overwrite gamepad from config
		{
			ButtonStruct* btn = &gaButtons[i][0];
			char entry[16];
			memcpy(&entry[0], i == 0 ? "menu" : "game", 4);
			entry[4] = '.';
			entry[7] = '.';
			int n = 0;
			while (true) {
				entry[5] = n > 9 ? '1' : '0';
				entry[6] = ('0' + 1) == '1' ? ('0' + n) : (n == 0 ? '0' : ('1' + n - 1));
				memcpy(&entry[8], "size", 5);
				int size = 0;
				if (!getIniInt("GamePad", entry, &size)) {
					break;
				}
				btn->buSize = size;
				if (size == 0) {
					break;
				}
				memcpy(&entry[8], "x", 2);
				getIniInt("GamePad", entry, &btn->buPosX);
				memcpy(&entry[8], "y", 2);
				getIniInt("GamePad", entry, &btn->buPosY);
				memcpy(&entry[8], "key", 4);
				getIniInt("GamePad", entry, &btn->buVkCode);
				memcpy(&entry[8], "frame", 6);
				if (!getIniInt("GamePad", entry, &btn->buFrame)) {
					btn->buFrame = 1;
				}
				char label[2] = "";
				memcpy(&entry[8], "label", 6);
				getIniValue("GamePad", entry, label, 2);
				btn->buLabel = label[0];
				n++;
				btn++;
				if (n == MAX_GAMEPAD_BUTTONS) {
					break;
				}
			}
#if VIRTUAL_GAMEPAD
			if (n > 0 && n < MAX_GAMEPAD_BUTTONS) {
				btn->buSize = 0;
			}
#endif
		}
		// reset the state of the buttons
		ButtonStruct* btn = &gaButtons[i][0];
		while (true) {
			if (btn->buSize == 0) {
				break;
			}
			// btn->buPressed = false;
			for (int n = 0; n < MAX_GAMEPAD_FINGERS; n++) {
				btn->buFinger[n] = NO_TOUCH;
			}
			btn++;
		}
	}
}

void InitTouch()
{
	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			finger[port][i].id = NO_TOUCH;
		}
		static_assert(DRAG_NONE == 0, "InitTouch must initialize the drag-modes.");
		// multi_finger_dragging[port] = DRAG_NONE;
	}

#ifdef __vita__
	back_touch = getIniBool("Controller", "enable_second_touchscreen", true);
#endif
	init_virtual_gamepad();
}

void InitGamepadGFX()
{
	assert(pGamepadIconsCels == NULL);
	pGamepadIconsCels = CelLoadImage("Data\\TouchBtn.CEL", 0);
}

void FreeGamepadGFX()
{
	MemFreeDbg(pGamepadIconsCels);
}

static void preprocess_indirect_finger_down(SDL_Event* event)
{
	event->type        = SDL_CONTROLLERAXISMOTION;
	event->caxis.value = 32767;
	event->caxis.which = 0;
	event->caxis.axis  = event->tfinger.x <= 0.5 ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}

static void preprocess_indirect_finger_up(SDL_Event* event)
{
	event->type        = SDL_CONTROLLERAXISMOTION;
	event->caxis.value = 0;
	event->caxis.which = 0;
	event->caxis.axis  = event->tfinger.x <= 0.5 ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}

static void TouchToLogical(const SDL_Event* event, int& x, int& y)
{
	x = event->tfinger.x * SCREEN_WIDTH;
	y = event->tfinger.y * SCREEN_HEIGHT;
}

static void preprocess_direct_finger_down(const SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	const SDL_FingerID id = event->tfinger.fingerId;

	int numFingersDown = 0;
	// int fingerIdx = -1;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			//if (finger[port][i].id == id) {
			//	fingerIdx = i;
			//}
			numFingersDown++;
		}
	}

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			// make sure each finger is not reported down multiple times
			if (finger[port][i].id != id) {
				continue;
			}
		}
		finger[port][i].id             = id;
		// preserve the timestamp to calculate the tap-length
		finger[port][i].time_last_down = event->tfinger.timestamp;
		int x, y;
		TouchToLogical(event, x, y);
		// remember the starting coordinates for multi-finger drag
		finger[port][i].first_x        = x;
		finger[port][i].first_y        = y;
		// remember the last coordinates to keep track of dragging
		finger[port][i].last_x         = x;
		finger[port][i].last_y         = y;
		// update the mouse-position on first touch
		if (numFingersDown == 0) {
			SDL_Event ev;
			SetMouseMotionEvent(&ev, x, y);
			SDL_PushEvent(&ev);
		}
		// reset DRAG_OVER states to allow sequent 'right' clicks while holding the other finger down
		multi_finger_dragging[port] = DRAG_NONE;
		break;
	}
}

static void preprocess_direct_finger_up(const SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	const SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	int fingerIdx = -1;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			if (finger[port][i].id == id) {
				fingerIdx = i;
			}
			numFingersDown++;
		}
	}

	if (fingerIdx >= 0) {
		finger[port][fingerIdx].id = NO_TOUCH;
		if (multi_finger_dragging[port] == DRAG_NONE) {
			if (SDL_TICKS_PASSED(event->tfinger.timestamp, finger[port][fingerIdx].time_last_down + MAX_TAP_TIME)) {
				return; // continue;
			}

			// assert(numFingersDown >= 1);
			int simulatedBtnIdx = numFingersDown - 1;
			if (simulatedBtnIdx >= TOUCH_PORT_CLICK_NUM) {
				return; // continue;
			}
			// ensure the other button is not triggering any more
			if (simulatedBtnIdx != 0) {
				multi_finger_dragging[port] = DRAG_OVER;
			}
			int x, y;
			TouchToLogical(event, x, y);
			static_assert(TOUCH_PORT_CLICK_NUM == 2, "preprocess_direct_finger_up is limited to 2 simulated button-types I.");
			Uint8 simulatedButton = simulatedBtnIdx == 0 ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
			SDL_Event ev;
			SetMouseMotionEvent(&ev, x, y);
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONDOWN, simulatedButton, SDL_PRESSED, x, y);
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, x, y);
			SDL_PushEvent(&ev);
		} else if (numFingersDown == 1) {
			// when dragging, and the last finger is lifted, the drag is over
			int x = MousePos.x;
			int y = MousePos.y;
			static_assert(TOUCH_PORT_CLICK_NUM == 2, "preprocess_direct_finger_up is limited to 2 simulated button-types II.");
			Uint8 simulatedButton = multi_finger_dragging[port] == DRAG_TWO_FINGER ? SDL_BUTTON_LEFT : (multi_finger_dragging[port] == DRAG_THREE_FINGER ? SDL_BUTTON_RIGHT : 0);
			multi_finger_dragging[port] = DRAG_NONE;
			if (simulatedButton != 0) {
				SDL_Event ev;
				SetMouseMotionEvent(&ev, x, y);
				SDL_PushEvent(&ev);
				SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, x, y);
				SDL_PushEvent(&ev);
			}
		}
	}
}

static int first_direct_finger_index()
{
	const int port = 0;
	Uint32 earliestTime;
	int firstIdx = -1;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id == NO_TOUCH) {
			continue;
		}
		if (firstIdx < 0 || !SDL_TICKS_PASSED(finger[port][i].time_last_down, earliestTime)) {
			firstIdx = i;
			earliestTime = finger[port][i].time_last_down;
		}
	}
	return firstIdx;
}

static void preprocess_direct_finger_motion(const SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	const SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	int fingerIdx = -1;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			if (finger[port][i].id == id) {
				fingerIdx = i;
			}
			numFingersDown++;
		}
	}

	if (fingerIdx >= 0) {
		// If we are starting a multi-finger drag, start holding down the mouse button
		if (numFingersDown >= 2 && multi_finger_dragging[port] == DRAG_NONE) {
			// only start a multi-finger drag if at least two fingers have been down long enough
			int numFingersDownlong = 0;
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (finger[port][i].id == NO_TOUCH) {
					continue;
				}
				if (SDL_TICKS_PASSED(event->tfinger.timestamp, finger[port][i].time_last_down + MAX_TAP_TIME)) {
					numFingersDownlong++;
				}
			}
			if (numFingersDownlong >= 2) {
				int firstIdx = first_direct_finger_index();
				int x = finger[port][firstIdx].first_x;
				int y = finger[port][firstIdx].first_y;

				Uint8 simulatedButton;
				if (numFingersDownlong == 2) {
					simulatedButton = SDL_BUTTON_LEFT;
					multi_finger_dragging[port] = DRAG_TWO_FINGER;
				} else {
					simulatedButton = SDL_BUTTON_RIGHT;
					multi_finger_dragging[port] = DRAG_THREE_FINGER;
				}
				SDL_Event ev;
				SetMouseMotionEvent(&ev, x, y);
				SDL_PushEvent(&ev);
				SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONDOWN, simulatedButton, SDL_PRESSED, x, y);
				SDL_PushEvent(&ev);
			}
		}
		int x, y, xrel, yrel;
		TouchToLogical(event, x, y);

		xrel = x - finger[port][fingerIdx].last_x;
		yrel = y - finger[port][fingerIdx].last_y;
		if (xrel == 0 && yrel == 0) {
			return;
		}

		// update the current finger's coordinates so we can track it later
		finger[port][fingerIdx].last_x = x;
		finger[port][fingerIdx].last_y = y;

		// check if this is the "oldest" finger down (or the only finger down)
		// otherwise it will not affect mouse motion
		bool updatePointer = true;
		if (numFingersDown > 1) {
			int firstIdx = first_direct_finger_index();
			updatePointer = firstIdx == fingerIdx;
		}
		if (!updatePointer) {
			return;
		}
		SDL_Event ev;
		SetMouseMotionEvent(&ev, x, y);
		SDL_PushEvent(&ev);
	}
}

static void SetKeyPressEvent(SDL_Event* evt, int evt_type, int code)
{
	evt->type = SDL_USEREVENT;
	evt->user.code = evt_type;
	evt->vkcode = code;
}

static bool preprocess_gamepad_down(ButtonStruct* btn, SDL_Event* event)
{
	int x, y;
	TouchToLogical(event, x, y);

	// id (for multitouch)
	const SDL_FingerID id = event->tfinger.fingerId;

	while (true) {
		if (btn->buSize == 0) {
			break;
		}
		if (POS_IN_RECT(x, y, btn->buPosX, btn->buPosY - btn->buSize, btn->buSize, btn->buSize)) {
			int numFingersDown = 0;
			int n = -1;
			for (int i = MAX_GAMEPAD_FINGERS - 1; i >= 0; i--) {
				if (btn->buFinger[i] != NO_TOUCH) {
					numFingersDown++;
				} else {
					n = i;
				}
			}
			if (n >= 0) {
				btn->buFinger[n] = id;
			}
			if (numFingersDown == 0) {
				btn->buPressed = true;
				SetKeyPressEvent(event, DVL_WM_KEYDOWN, btn->buVkCode);
			}
			return true;
		}
		btn++;
	}
	return false;
}

static ButtonStruct* preprocess_gamepad_button(ButtonStruct* btn, SDL_Event* event)
{
	// id (for multitouch)
	const SDL_FingerID id = event->tfinger.fingerId;

	while (true) {
		if (btn->buSize == 0) {
			break;
		}
		for (int i = 0; i < MAX_GAMEPAD_FINGERS; i++) {
			if (btn->buFinger[i] == id) {
				return btn;
			}
		}
		btn++;
	}
	return NULL;
}

static bool preprocess_gamepad_motion(ButtonStruct* btn, SDL_Event* event)
{
	btn = preprocess_gamepad_button(btn, event);
	return btn != NULL;
}

static bool preprocess_gamepad_up(ButtonStruct* btn, SDL_Event* event)
{
	btn = preprocess_gamepad_button(btn, event);

	bool result = btn != NULL;
	if (result) {
		// id (for multitouch)
		const SDL_FingerID id = event->tfinger.fingerId;
		int numFingersDown = 0;
		for (int i = 0; i < MAX_GAMEPAD_FINGERS; i++) {
			if (btn->buFinger[i] == id) {
				btn->buFinger[i] = NO_TOUCH;
			} else if (btn->buFinger[i] != NO_TOUCH) {
				numFingersDown++;
			}
		}
		btn->buPressed = numFingersDown != 0;

		if (numFingersDown == 0) {
			SetKeyPressEvent(event, DVL_WM_KEYUP, btn->buVkCode);
		}

	}
	return result;
}

static bool TryPadClick(SDL_Event* event)
{
	bool result = false;
	ButtonStruct* btn = gaButtons[gbRunGame ? 1 : 0];
	switch (event->type) {
	case SDL_FINGERDOWN:
		result = preprocess_gamepad_down(btn, event);
		break;
	case SDL_FINGERUP:
		result = preprocess_gamepad_up(btn, event);
		break;
	case SDL_FINGERMOTION:
		result = preprocess_gamepad_motion(btn, event);
		break;
	default:
		ASSUME_UNREACHABLE
	}
	return result;
}

static void PreprocessEvents(SDL_Event* event)
{
	// Supported touch gestures:
	// left mouse click: single finger short tap
	// right mouse click: second finger short tap while first finger is still down
	// pointer motion: single finger drag
	// left button drag and drop: dual finger drag
	// right button drag and drop: triple finger drag
	if (event->type != SDL_FINGERDOWN && event->type != SDL_FINGERUP && event->type != SDL_FINGERMOTION) {
		return;
	}

	SDL_TouchDeviceType devType = SDL_GetTouchDeviceType(event->tfinger.touchId);
	if (devType != SDL_TOUCH_DEVICE_DIRECT) {
#ifdef __vita__
		if (!back_touch) {
			return;
		}
#endif
		if (devType == SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE) {
			switch (event->type) {
			case SDL_FINGERDOWN:
				preprocess_indirect_finger_down(event);
				break;
			case SDL_FINGERUP:
				preprocess_indirect_finger_up(event);
				break;
			case SDL_FINGERMOTION:
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
		}
		return;
	}

	if (TryPadClick(event)) {
		return;
	}

	switch (event->type) {
	case SDL_FINGERDOWN:
		preprocess_direct_finger_down(event);
		break;
	case SDL_FINGERUP:
		preprocess_direct_finger_up(event);
		break;
	case SDL_FINGERMOTION:
		preprocess_direct_finger_motion(event);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void handle_touch(SDL_Event* event)
{
	PreprocessEvents(event);
}

void DrawGamepad()
{
	ButtonStruct* btn = gaButtons[gbRunGame ? 1 : 0];
	while (true) {
		if (btn->buSize == 0) {
			break;
		}
		pGamepadIconsCels->ciWidth = btn->buSize;
		int sx = SCREEN_X + btn->buPosX;
		int sy = SCREEN_Y + btn->buPosY;
		CelDrawTrnTbl(sx, sy, pGamepadIconsCels, btn->buFrame, ColorTrns[!btn->buPressed ? 0 : 3]);

		if (btn->buLabel != 0) {
			char text[2] = { btn->buLabel, '\0' };
			PrintString(AFF_HUGE | AFF_HCENTER | AFF_VCENTER | UIS_GOLD, text, sx, sy - btn->buSize + 2, btn->buSize, btn->buSize);
		}
		btn++;
	}
}

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
