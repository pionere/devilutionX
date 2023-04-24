#include "touch.h"
#if HAS_TOUCHPAD
#include "utils/display.h"
#include "../gameui.h"
#include "../diablo.h"
#include <math.h>

#ifdef __vita__
#include "storm/storm.h"
static bool back_touch = false;
#endif

DEVILUTION_BEGIN_NAMESPACE

static int visible_width;
static int visible_height;
static int x_borderwidth;
static int y_borderwidth;

template <typename T>
inline T clip(T v, T amin, T amax)
{
	if (v < amin)
		return amin;
	else if (v > amax)
		return amax;
	else
		return v;
}

#define TOUCH_PORT_MAX_NUM   1
#define TOUCH_PORT_CLICK_NUM 2
// finger id setting if finger is not touching the screen
#define NO_TOUCH (-1)
// number of fingers to track per panel
#define MAX_NUM_FINGERS 3
// taps longer than this will not result in mouse click events (ms)
#define MAX_TAP_TIME 250
// max distance finger motion in Vita screen pixels to be considered a tap
#define MAX_TAP_MOTION_DISTANCE 10
// duration of a simulated mouse click (ms)
#define SIMULATED_CLICK_DURATION 50

static bool touch_initialized = false;
// initiation time of last simulated left or right click (zero if no click)
static unsigned int simulated_click_start_time[TOUCH_PORT_MAX_NUM][TOUCH_PORT_CLICK_NUM];
// pointer jumps to finger
static bool direct_touch = true;

struct Touch {
	SDL_FingerID id; // -1: not touching
	uint32_t time_last_down;
	int last_x;        // last known screen coordinates
	int last_y;        // last known screen coordinates
	float last_down_x; // SDL touch coordinates when last pressed down
	float last_down_y; // SDL touch coordinates when last pressed down
};

static Touch finger[TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS]; // keep track of finger status

enum DraggingType {
	DRAG_NONE,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
};

static DraggingType multi_finger_dragging[TOUCH_PORT_MAX_NUM]; // keep track whether we are currently drag-and-dropping

static void SetMouseButtonEvent(SDL_Event* event, uint32_t type, uint8_t button, int32_t x, int32_t y)
{
	event->type = type;
	event->button.button = button;
	if (type == SDL_MOUSEBUTTONDOWN) {
		event->button.state = SDL_PRESSED;
	} else {
		event->button.state = SDL_RELEASED;
	}
	event->button.x = x;
	event->button.y = y;
}

static void SetMouseMotionEvent(SDL_Event* event, int32_t x, int32_t y, int32_t xrel, int32_t yrel)
{
	event->type = SDL_MOUSEMOTION;
	event->motion.x = x;
	event->motion.y = y;
	event->motion.xrel = xrel;
	event->motion.yrel = yrel;
}

static void InitTouch()
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

	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	visible_height = current.h;
	visible_width = (current.h * SCREEN_WIDTH) / SCREEN_HEIGHT;
	x_borderwidth = (current.w - visible_width) / 2;
	y_borderwidth = (current.h - visible_height) / 2;
#ifdef __vita__
	back_touch = dvl::getIniBool("Controller", "enable_second_touchscreen", true);
#endif
}

#ifdef __vita__
static void preprocess_back_finger_down(SDL_Event* event)
{
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;

	if (port != 1)
		return;

	event->type        = SDL_CONTROLLERAXISMOTION;
	event->caxis.value = 32767;
	event->caxis.which = 0;
	event->caxis.axis  = event->tfinger.x <= 0.5 ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}

static void preprocess_back_finger_up(SDL_Event* event)
{
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;

	if (port != 1)
		return;

	event->type        = SDL_CONTROLLERAXISMOTION;
	event->caxis.value = 0;
	event->caxis.which = 0;
	event->caxis.axis  = event->tfinger.x <= 0.5 ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}
#endif

static void TouchToLogical(SDL_Event* event, int& x, int& y)
{
	x = (int)(event->tfinger.x * visible_width + x_borderwidth);
	y = (int)(event->tfinger.y * visible_height + y_borderwidth);
	dvl::OutputToLogical(&x, &y);
}

static void PreprocessFingerDown(SDL_Event* event)
{
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	int x = MousePos.x;
	int y = MousePos.y;

	if (direct_touch) {
		TouchToLogical(event, x, y);
	}

	// make sure each finger is not reported down multiple times
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != id) {
			continue;
		}
		finger[port][i].id = NO_TOUCH;
	}

	// we need the timestamps to decide later if the user performed a short tap (click)
	// or a long tap (drag)
	// we also need the last coordinates for each finger to keep track of dragging
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			continue;
		}
		finger[port][i].id             = id;
		finger[port][i].time_last_down = event->tfinger.timestamp;
		finger[port][i].last_down_x    = event->tfinger.x;
		finger[port][i].last_down_y    = event->tfinger.y;
		finger[port][i].last_x         = x;
		finger[port][i].last_y         = y;
		break;
	}
}

static void PreprocessFingerUp(SDL_Event* event)
{
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			numFingersDown++;
		}
	}

	int x = MousePos.x;
	int y = MousePos.y;

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != id) {
			continue;
		}

		finger[port][i].id = NO_TOUCH;
		if (multi_finger_dragging[port] == DRAG_NONE) {
			if ((event->tfinger.timestamp - finger[port][i].time_last_down) > MAX_TAP_TIME) {
				continue;
			}

			// short (<MAX_TAP_TIME ms) tap is interpreted as right/left mouse click depending on # fingers already down
			// but only if the finger hasn't moved since it was pressed down by more than MAX_TAP_MOTION_DISTANCE pixels
			float xrel = ((event->tfinger.x * dvl::GetOutputSurface()->w) - (finger[port][i].last_down_x * dvl::GetOutputSurface()->w));
			float yrel = ((event->tfinger.y * dvl::GetOutputSurface()->h) - (finger[port][i].last_down_y * dvl::GetOutputSurface()->h));
			float maxRSquared = (float)(MAX_TAP_MOTION_DISTANCE * MAX_TAP_MOTION_DISTANCE);
			if ((xrel * xrel + yrel * yrel) >= maxRSquared) {
				continue;
			}

			if (numFingersDown != 2 && numFingersDown != 1) {
				continue;
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
				if (direct_touch) {
					TouchToLogical(event, x, y);
				}
			}
			SetMouseButtonEvent(event, SDL_MOUSEBUTTONDOWN, simulatedButton, x, y);
		} else if (numFingersDown == 1) {
			// when dragging, and the last finger is lifted, the drag is over
			Uint8 simulatedButton = 0;
			if (multi_finger_dragging[port] == DRAG_THREE_FINGER) {
				simulatedButton = SDL_BUTTON_RIGHT;
			} else {
				simulatedButton = SDL_BUTTON_LEFT;
			}
			SetMouseButtonEvent(event, SDL_MOUSEBUTTONUP, simulatedButton, x, y);
			multi_finger_dragging[port] = DRAG_NONE;
		}
	}
}

static void PreprocessFingerMotion(SDL_Event* event)
{
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (finger[port][i].id != NO_TOUCH) {
			numFingersDown++;
		}
	}

	if (numFingersDown != 0) {
		int x, y, xrel, yrel;

		if (direct_touch) {
			TouchToLogical(event, x, y);
		} else {
			// for relative mode, use the pointer speed setting
			float speedFactor = 1.0;

			// convert touch events to relative mouse pointer events
			// Whenever an SDL_event involving the mouse is processed,
			x = MousePos.x + (int)(event->tfinger.dx * 1.25 * speedFactor * dvl::GetOutputSurface()->w);
			y = MousePos.y + (int)(event->tfinger.dy * 1.25 * speedFactor * dvl::GetOutputSurface()->h);
		}

		x = clip(x, 0, dvl::GetOutputSurface()->w);
		y = clip(y, 0, dvl::GetOutputSurface()->h);
		xrel = x - MousePos.x;
		yrel = y - MousePos.y;

		// update the current finger's coordinates so we can track it later
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			if (finger[port][i].id != id)
				continue;
			finger[port][i].last_x = x;
			finger[port][i].last_y = y;
		}

		// If we are starting a multi-finger drag, start holding down the mouse button
		if (numFingersDown >= 2 && multi_finger_dragging[port] == DRAG_NONE) {
			// only start a multi-finger drag if at least two fingers have been down long enough
			int numFingersDownlong = 0;
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (finger[port][i].id == NO_TOUCH) {
					continue;
				}
				if (event->tfinger.timestamp - finger[port][i].time_last_down > MAX_TAP_TIME) {
					numFingersDownlong++;
				}
			}
			if (numFingersDownlong >= 2) {
				int mouseDownX = MousePos.x;
				int mouseDownY = MousePos.y;
				if (direct_touch) {
					for (int i = 0; i < MAX_NUM_FINGERS; i++) {
						if (finger[port][i].id == id) {
							uint32_t earliestTime = finger[port][i].time_last_down;
							for (int j = 0; j < MAX_NUM_FINGERS; j++) {
								if (finger[port][j].id != NO_TOUCH && (i != j)) {
									if (finger[port][j].time_last_down < earliestTime) {
										mouseDownX = finger[port][j].last_x;
										mouseDownY = finger[port][j].last_y;
										earliestTime = finger[port][j].time_last_down;
									}
								}
							}
							break;
						}
					}
				}

				Uint8 simulatedButton = 0;
				if (numFingersDownlong == 2) {
					simulatedButton = SDL_BUTTON_LEFT;
					multi_finger_dragging[port] = DRAG_TWO_FINGER;
				} else {
					simulatedButton = SDL_BUTTON_RIGHT;
					multi_finger_dragging[port] = DRAG_THREE_FINGER;
				}
				SDL_Event ev;
				SetMouseMotionEvent(&ev, mouseDownX, mouseDownY, 0, 0); // TODO: xrel/yrel?
				SDL_PushEvent(&ev);
				SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONDOWN, simulatedButton, mouseDownX, mouseDownY);
				SDL_PushEvent(&ev);
			}
		}

		if (xrel == 0 && yrel == 0) {
			return;
		}

		// check if this is the "oldest" finger down (or the only finger down)
		// otherwise it will not affect mouse motion
		bool updatePointer = true;
		if (numFingersDown > 1) {
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (finger[port][i].id != id) {
					continue;
				}
				for (int j = 0; j < MAX_NUM_FINGERS; j++) {
					if (finger[port][j].id == NO_TOUCH || (j == i)) {
						continue;
					}
					if (finger[port][j].time_last_down < finger[port][i].time_last_down) {
						updatePointer = false;
					}
				}
			}
		}
		if (!updatePointer) {
			return;
		}
		SetMouseMotionEvent(event, x, y, xrel, yrel);
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

	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	if (port != 0) {
#ifdef __vita__
		if (back_touch) {
			switch (event->type) {
			case SDL_FINGERDOWN:
				preprocess_back_finger_down(event);
				break;
			case SDL_FINGERUP:
				preprocess_back_finger_up(event);
				break;
			}
		}
#endif
		return;
	}

	switch (event->type) {
	case SDL_FINGERDOWN:
		PreprocessFingerDown(event);
		break;
	case SDL_FINGERUP:
		PreprocessFingerUp(event);
		break;
	case SDL_FINGERMOTION:
		PreprocessFingerMotion(event);
		break;
	}
}

void handle_touch(SDL_Event* event)
{
	if (!touch_initialized) {
		InitTouch();
		touch_initialized = true;
	}
	PreprocessEvents(event);
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		event->type = SDL_USEREVENT;
		event->user.code = -1; // ensure this event is ignored;
	}
}

void finish_simulated_mouse_clicks()
{
	int mouse_x = MousePos.x;
	int mouse_y = MousePos.y;

	for (int port = 0; port < TOUCH_PORT_MAX_NUM; port++) {
		for (int i = 0; i < TOUCH_PORT_CLICK_NUM; i++) {
			if (simulated_click_start_time[port][i] == 0) {
				continue;
			}

			Uint32 currentTime = SDL_GetTicks();
			if (currentTime - simulated_click_start_time[port][i] < SIMULATED_CLICK_DURATION) {
				continue;
			}

			int simulatedButton;
			if (i == 0) {
				simulatedButton = SDL_BUTTON_LEFT;
			} else {
				simulatedButton = SDL_BUTTON_RIGHT;
			}
			SDL_Event ev;
			SetMouseMotionEvent(&ev, mouse_x, mouse_y, 0, 0); // TODO: xrel/yrel?
			SDL_PushEvent(&ev);
			SetMouseButtonEvent(&ev, SDL_MOUSEBUTTONUP, simulatedButton, mouse_x, mouse_y);
			SDL_PushEvent(&ev);

			simulated_click_start_time[port][i] = 0;
		}
	}
}

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
