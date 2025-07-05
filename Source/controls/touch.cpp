#include "touch.h"
#if HAS_TOUCHPAD
#include "engine/render/cel_render.h"
#include "utils/display.h"
#include "utils/sdl2_backports.h"
#include "../gameui.h"
#include "../diablo.h"
#include "../miniwin/miniwin.h"
#include "../plrmsg.h"
#include <math.h>

#ifdef __vita__
#include "storm/storm_cfg.h"
static bool back_touch = false;
#endif

DEVILUTION_BEGIN_NAMESPACE

static void LogErrorFFFF(const char* msg, ...)
{
	char tmp[256];

	FILE* f0 = NULL;
	while (f0 == NULL) {
		f0 = fopen("/storage/0403-0201/Android/data/org.diasurgical.devilx/files/logdebug0.txt", "a+");
	}

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	fputs(tmp, f0);

	fputc('\n', f0);

	fclose(f0);
}

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
// max distance finger motion in screen pixels to be considered a tap
//#define MAX_TAP_MOTION_DISTANCE 10
// duration of a simulated mouse click (ms)
// #define SIMULATED_CLICK_DURATION 50
#define MAX_GAMEPAD_BUTTONS 16

// initiation time of last simulated left or right click (zero if no click)
//static Uint32 simulated_click_start_time[TOUCH_PORT_MAX_NUM][TOUCH_PORT_CLICK_NUM];

typedef struct FingerStruct {
	SDL_FingerID id; // -1: not touching
	Uint32 time_last_down;
	int last_x;        // last known screen x-coordinate
	int last_y;        // last known screen y-coordinate
	//float last_down_x; // SDL touch coordinates when last pressed down
	//float last_down_y; // SDL touch coordinates when last pressed down
	int first_x; // first screen x-coordinate
	int first_y; // first screen y-coordinate
} FingerStruct;

static FingerStruct finger[TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS]; // keep track of finger status

enum DraggingType {
	DRAG_NONE,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
	DRAG_OVER,
};

static DraggingType multi_finger_dragging[TOUCH_PORT_MAX_NUM]; // keep track whether we are currently drag-and-dropping

/** touchpad icons  CEL */
static CelImageBuf* pTouchIconsCels;

typedef struct ButtonStruct {
	int buType;
	int buAction;
	int buFrame;
	int buPosX;
	int buPosY;
	int buSize;

	bool buPressed;
} ButtonStruct;

static ButtonStruct gaButtons[MAX_GAMEPAD_BUTTONS];
static unsigned numButtons;

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
	numButtons = 12;

	gaButtons[0] = { 0, ACT_SKL0, 5, 0, 90, 87, false };
	gaButtons[1] = { 0, ACT_SKL1, 5, 90, 90, 87, false };
	gaButtons[2] = { 0, ACT_SKL2, 5, 0, 180, 87, false };
	gaButtons[3] = { 0, ACT_SKL3, 5, 90, 180, 87, false };

	gaButtons[4] = { 0, ACT_SKL4, 5, 780, 90, 87, false };
	gaButtons[5] = { 0, ACT_SKL5, 5, 870, 90, 87, false };
	gaButtons[6] = { 0, ACT_SKL6, 5, 780, 180, 87, false };
	gaButtons[7] = { 0, ACT_SKL7, 5, 870, 180, 87, false };

	gaButtons[8] = { 1, NUM_ACTS/*ACT_RLACT?*/, 11, 0, 90, 87, false };
	gaButtons[9] = { 1, 0/*ACT_STANDACT*/, 12, 90, 90, 87, false };

	gaButtons[10] = { 1, 1/*ACT_MODACT*/, 6, 0, 180, 87, false };
	gaButtons[11] = { 1, 0/*ACT_STANDACT*/, 12, 90, 180, 87, false };
}

void InitTouch()
{
	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			finger[port][i].id = NO_TOUCH;
		}
		multi_finger_dragging[port] = DRAG_NONE;
	}

	/*for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < TOUCH_PORT_CLICK_NUM; i++) {
			simulated_click_start_time[port][i] = 0;
		}
	}*/

#ifdef __vita__
	back_touch = dvl::getIniBool("Controller", "enable_second_touchscreen", true);
#endif
	LogErrorF("initTouch 0");
	init_virtual_gamepad();
	LogErrorF("initTouch 1");
	assert(pTouchIconsCels == NULL);
	LogErrorF("initTouch 1.1");
	pTouchIconsCels = CelLoadImage("Data\\TouchBtn.CEL", 0);
	LogErrorF("initTouch 2");
}

void FreeTouch()
{
	MemFreeDbg(pTouchIconsCels);
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
#ifdef NOPOS
		finger[port][i].last_x         = -1;
		finger[port][i].last_y         = -1;
	// LogErrorFFFF("down: %d:%d idx: %d (%d) (%f:%f)", x, y, i, id, event->tfinger.x, event->tfinger.y);
	// EventPlrMsg("down: %d:%d idx: %d (%d) (%f:%f)", x, y, i, id, event->tfinger.x, event->tfinger.y);
#else
		finger[port][i].last_x         = x;
		finger[port][i].last_y         = y;
		if (numFingersDown == 0) {
#ifdef ALTER_EVENTS
			SetMouseMotionEvent(event, x, y);
#else
			SDL_Event ev;
			SetMouseMotionEvent(&ev, x, y);
			SDL_PushEvent(&ev);
#endif
		}
#endif
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

	/*int xx = MousePos.x;
	int yy = MousePos.y;

	LogErrorFFFF("up: %d:%d idx: %d (%d) fd %d drag %d", xx, yy, fingerIdx, fingerIdx != NO_TOUCH ? finger[port][fingerIdx].id : 0, numFingersDown, multi_finger_dragging[port]);
	EventPlrMsg("up: %d:%d idx: %d (%d) fd %d drag %d", xx, yy, fingerIdx, fingerIdx != NO_TOUCH ? finger[port][fingerIdx].id : 0, numFingersDown, multi_finger_dragging[port]);*/
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
			// need to raise the button later
			// simulated_click_start_time[port][simulatedBtnIdx] = event->tfinger.timestamp;
			static_assert(TOUCH_PORT_CLICK_NUM == 2, "preprocess_direct_finger_up is limited to 2 simulated button-types");
			int x, y;
			TouchToLogical(event, x, y);
			LogErrorFFFF("click%d: %d:%d", simulatedBtnIdx, x, y);
			EventPlrMsg("click%d: %d:%d", simulatedBtnIdx, x, y);
#ifdef DISPATCH_TOUCH
			MousePos.x = x;
			MousePos.y = y;
			Dvl_Event ev;
			ev.button.x = x;
			ev.button.y = y;
			ev.type = simulatedBtnIdx == 0 ? DVL_WM_LBUTTONDOWN : DVL_WM_RBUTTONDOWN;
			DispatchMessage(&ev);
			ev.type = simulatedBtnIdx == 0 ? DVL_WM_LBUTTONUP : DVL_WM_RBUTTONUP;
			DispatchMessage(&ev);
#else
			Uint8 simulatedButton = simulatedBtnIdx == 0 ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
			SDL_Event ev;
			SetMouseMotionEvent(&ev, x, y);
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONDOWN, simulatedButton, SDL_PRESSED, x, y);
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, x, y);
			SDL_PushEvent(&ev);
#endif
		} else if (numFingersDown == 1) {
			// when dragging, and the last finger is lifted, the drag is over
			int x = MousePos.x;
			int y = MousePos.y;
#ifdef DISPATCH_TOUCH
			Dvl_Event ev;
			ev.button.x = x;
			ev.button.y = y;
			ev.type = multi_finger_dragging[port] == DRAG_TWO_FINGER ? DVL_WM_LBUTTONUP : DVL_WM_RBUTTONUP;
			multi_finger_dragging[port] = DRAG_NONE;
			DispatchMessage(&ev);
#else
			Uint8 simulatedButton = multi_finger_dragging[port] == DRAG_TWO_FINGER ? SDL_BUTTON_LEFT : (multi_finger_dragging[port] == DRAG_THREE_FINGER ? SDL_BUTTON_RIGHT : 0);
			multi_finger_dragging[port] = DRAG_NONE;
			if (simulatedButton != 0) {
				SDL_Event ev;
				SetMouseMotionEvent(&ev, x, y);
				SDL_PushEvent(&ev);
				SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, x, y);
				SDL_PushEvent(&ev);
			}
#endif
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
		int x, y, xrel, yrel;
#ifdef NOPOS
		TouchToLogical(event, x, y);

		//xrel = x - MousePos.x;
		//yrel = y - MousePos.y;
		xrel = x - finger[port][fingerIdx].last_x;
		yrel = y - finger[port][fingerIdx].last_y;
		if (xrel == 0 && yrel == 0) {
			return;
		}

		// update the current finger's coordinates so we can track it later
			finger[port][fingerIdx].last_x = x;
			finger[port][fingerIdx].last_y = y;
#endif
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
				int mouseDownX = finger[port][firstIdx].first_x;
				int mouseDownY = finger[port][firstIdx].first_y;
	// LogErrorFFFF("sim down: %d:%d lr: %d", mouseDownX, mouseDownY, numFingersDownlong == 2);
	// EventPlrMsg("sim down: %d:%d lr: %d", mouseDownX, mouseDownY, numFingersDownlong == 2);
				// Uint8 simulatedButton = 0;
				if (numFingersDownlong == 2) {
					// simulatedButton = SDL_BUTTON_LEFT;
					multi_finger_dragging[port] = DRAG_TWO_FINGER;
				} else {
					// simulatedButton = SDL_BUTTON_RIGHT;
					multi_finger_dragging[port] = DRAG_THREE_FINGER;
				}
#ifdef DISPATCH_TOUCH
				MousePos.x = mouseDownX;
				MousePos.y = mouseDownY;
				Dvl_Event ev;
				ev.button.x = mouseDownX;
				ev.button.y = mouseDownY;
				ev.type = multi_finger_dragging[port] == DRAG_TWO_FINGER ? DVL_WM_LBUTTONDOWN : DVL_WM_RBUTTONDOWN;
				DispatchMessage(&ev);
#else
				Uint8 simulatedButton = multi_finger_dragging[port] == DRAG_TWO_FINGER ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
				SDL_Event ev;
				SetMouseMotionEvent(&ev, mouseDownX, mouseDownY);
				SDL_PushEvent(&ev);
				SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONDOWN, simulatedButton, SDL_PRESSED, mouseDownX, mouseDownY);
				SDL_PushEvent(&ev);
#endif
			}
		}
#ifndef NOPOS
		TouchToLogical(event, x, y);

		//xrel = x - MousePos.x;
		//yrel = y - MousePos.y;
		xrel = x - finger[port][fingerIdx].last_x;
		yrel = y - finger[port][fingerIdx].last_y;
		if (xrel == 0 && yrel == 0) {
			return;
		}

		// update the current finger's coordinates so we can track it later
			finger[port][fingerIdx].last_x = x;
			finger[port][fingerIdx].last_y = y;
#endif
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
	// LogErrorFFFF("move: %d:%d idx: %d (%d) (%f:%f)", x, y, fingerIdx, id, event->tfinger.x, event->tfinger.y);
	// EventPlrMsg("move: %d:%d idx: %d (%d) (%f:%f)", x, y, fingerIdx, id, event->tfinger.x, event->tfinger.y);
#ifdef ALTER_EVENTS
		SetMouseMotionEvent(event, x, y);
#else
		SDL_Event ev;
		SetMouseMotionEvent(&ev, x, y);
		SDL_PushEvent(&ev);
#endif
	}
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

	SDL_TouchID port = event->tfinger.touchId;
	{
		int x, y;
		TouchToLogical(event, x, y);
		LogErrorFFFF("touch event: %s on %d at %d:%d (%f:%f)", event->type == SDL_FINGERDOWN ? "d" : (event->type == SDL_FINGERUP ? "u" : "m"), port, x, y, event->tfinger.x, event->tfinger.y);
		// EventPlrMsg("touch event: %s on %d at %d:%d (%f:%f)", event->type == SDL_FINGERDOWN ? "d" : (event->type == SDL_FINGERUP ? "u" : "m"), port, x, y, event->tfinger.x, event->tfinger.y);
	}
	SDL_TouchDeviceType devType = SDL_GetTouchDeviceType(port);
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
			}
		}
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
	}
}

void handle_touch(SDL_Event* event)
{
	PreprocessEvents(event);
}

void DrawGamepad()
{
	for (unsigned i = 0; i < numButtons; i++) {
		ButtonStruct& btn = gaButtons[i];
		pTouchIconsCels->ciWidth = btn.buSize;
		CelDrawTrnTbl(SCREEN_X + btn.buPosX, SCREEN_Y + btn.buPosY, pTouchIconsCels, btn.buFrame, ColorTrns[!btn.buPressed ? 0 : 3]);
	}
}

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
