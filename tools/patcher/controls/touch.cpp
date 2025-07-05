#include "touch.h"
#if HAS_TOUCHPAD
#include "utils/display.h"
#include "utils/sdl2_backports.h"
#include "../gameui.h"
#include "../diablo.h"
#include <math.h>

#ifdef __vita__
#include "storm/storm_cfg.h"
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
// duration of a simulated mouse click (ms)
#define SIMULATED_CLICK_DURATION 50

// initiation time of last simulated left or right click (zero if no click)
static Uint32 simulated_click_start_time[TOUCH_PORT_MAX_NUM][TOUCH_PORT_CLICK_NUM];

struct Touch {
	SDL_FingerID id; // -1: not touching
	Uint32 time_last_down;
	int last_x;      // last known screen x-coordinate
	int last_y;      // last known screen y-coordinate
	int first_x;     // first screen x-coordinate
	int first_y;     // first screen y-coordinate
};

static Touch finger[TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS]; // keep track of finger status

enum DraggingType {
	DRAG_NONE,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
};

static DraggingType multi_finger_dragging[TOUCH_PORT_MAX_NUM]; // keep track whether we are currently drag-and-dropping

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

void InitTouch()
{
	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			finger[port][i].id = NO_TOUCH;
		}
		multi_finger_dragging[port] = DRAG_NONE;
	}

	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < TOUCH_PORT_CLICK_NUM; i++) {
			simulated_click_start_time[port][i] = 0;
		}
	}

#ifdef __vita__
	back_touch = dvl::getIniBool("Controller", "enable_second_touchscreen", true);
#endif
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

static void TouchToLogical(SDL_Event* event, int& x, int& y)
{
	x = event->tfinger.x * SCREEN_WIDTH;
	y = event->tfinger.y * SCREEN_HEIGHT;
}

static void preprocess_direct_finger_down(SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

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
		break;
	}
}

static void preprocess_direct_finger_up(SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

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

			if (numFingersDown != 2 && numFingersDown != 1) {
				return; // continue;
			}

			Uint8 simulatedButton = 0;
			if (numFingersDown == 2) {
				simulatedButton = SDL_BUTTON_RIGHT;
				// need to raise the button later
				simulated_click_start_time[port][1] = event->tfinger.timestamp;
			} else if (numFingersDown == 1) {
				simulatedButton = SDL_BUTTON_LEFT;
				// need to raise the button later
				simulated_click_start_time[port][0] = event->tfinger.timestamp;
			}
			int x, y;
			TouchToLogical(event, x, y);
			SetMouseButtonEvent(event, SDL_MOUSEBUTTONDOWN, simulatedButton, SDL_PRESSED, x, y);
		} else if (numFingersDown == 1) {
			// when dragging, and the last finger is lifted, the drag is over
			int x = MousePos.x;
			int y = MousePos.y;
			Uint8 simulatedButton = 0;
			if (multi_finger_dragging[port] == DRAG_THREE_FINGER) {
				simulatedButton = SDL_BUTTON_RIGHT;
			} else {
				simulatedButton = SDL_BUTTON_LEFT;
			}
			SetMouseButtonEvent(event, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, x, y);
			multi_finger_dragging[port] = DRAG_NONE;
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

static void preprocess_direct_finger_motion(SDL_Event* event)
{
	const int port = 0;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

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

				Uint8 simulatedButton = 0;
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
		SetMouseMotionEvent(event, x, y);
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

void finish_simulated_mouse_clicks()
{
	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < TOUCH_PORT_CLICK_NUM; i++) {
			if (simulated_click_start_time[port][i] == 0) {
				continue;
			}

			if (!SDL_TICKS_PASSED(SDL_GetTicks(), simulated_click_start_time[port][i] + SIMULATED_CLICK_DURATION))
				continue;

			simulated_click_start_time[port][i] = 0;

			int mouse_x = MousePos.x;
			int mouse_y = MousePos.y;

			int simulatedButton;
			if (i == 0) {
				simulatedButton = SDL_BUTTON_LEFT;
			} else {
				simulatedButton = SDL_BUTTON_RIGHT;
			}
			SDL_Event ev;
			SetMouseMotionEvent(&ev, mouse_x, mouse_y);
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, SDL_RELEASED, mouse_x, mouse_y);
			SDL_PushEvent(&ev);
		}
	}
}

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
