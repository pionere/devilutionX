/**
 * @file misc_msg.cpp
 *
 * Windows message handling and keyboard event conversion for SDL.
 */

#include "all.h"
#include "controls/controller.h"
#include "controls/controller_motion.h"
#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/game_controls.h"
#include "controls/touch.h"
#include "utils/display.h"

#ifdef __SWITCH__
#include "platform/switch/docking.h"
#include <switch.h>
#endif

DEVILUTION_BEGIN_NAMESPACE

/** The current input handler function */
WNDPROC CurrentWndProc;

#if __linux__ && (HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD)
#define FIX_WARPING 1
static bool mouseWarping = false;
static POS32 mouseWarpingPos;
#else
#define FIX_WARPING 0
#endif

void SetCursorPos(int x, int y)
{
#if FIX_WARPING
	mouseWarpingPos.x = x;
	mouseWarpingPos.y = y;
	mouseWarping = true;
#endif
	LogicalToOutput(&x, &y);
	SDL_WarpMouseInWindow(ghMainWnd, x, y);
}

static int TranslateSdlKey(SDL_Keysym key)
{
	// ref: https://wiki.libsdl.org/SDL_Keycode
	// ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#ifdef USE_SDL1
	SDLKey ksc = key.sym;
	switch (ksc) {
	case SDLK_BACKSPACE:
		return DVL_VK_BACK;
	case SDLK_TAB:
		return DVL_VK_TAB;
	case SDLK_CLEAR:
		return DVL_VK_OEM_CLEAR;
	case SDLK_RETURN:
		return DVL_VK_RETURN;
	case SDLK_PAUSE:
		return DVL_VK_PAUSE;
	case SDLK_ESCAPE:
		return DVL_VK_ESCAPE;
	case SDLK_SPACE:
		return DVL_VK_SPACE;
	case SDLK_EXCLAIM:
		return DVL_VK_OEM_10; // remap?
	case SDLK_QUOTEDBL:
		return DVL_VK_OEM_18; // remap?
	case SDLK_HASH:
		return DVL_VK_OEM_22; // remap?
	case SDLK_DOLLAR:
		return DVL_VK_OEM_20; // remap?
	case SDLK_AMPERSAND:
		return DVL_VK_OEM_17; // remap?
	case SDLK_QUOTE:
		return DVL_VK_OEM_7;
	case SDLK_LEFTPAREN:
		return DVL_VK_OEM_21; // remap?
	case SDLK_RIGHTPAREN:
		return DVL_VK_OEM_22; // remap?
	case SDLK_ASTERISK:
		return DVL_VK_MULTIPLY; // remap?
	case SDLK_PLUS:
		return DVL_VK_ADD;
	case SDLK_COMMA:
		return DVL_VK_OEM_COMMA;
	case SDLK_MINUS:
		return DVL_VK_OEM_MINUS;
	case SDLK_PERIOD:
		return DVL_VK_OEM_PERIOD;
	case SDLK_SLASH:
		return DVL_VK_OEM_2;
	case SDLK_0:
		return DVL_VK_0;
	case SDLK_1:
		return DVL_VK_1;
	case SDLK_2:
		return DVL_VK_2;
	case SDLK_3:
		return DVL_VK_3;
	case SDLK_4:
		return DVL_VK_4;
	case SDLK_5:
		return DVL_VK_5;
	case SDLK_6:
		return DVL_VK_6;
	case SDLK_7:
		return DVL_VK_7;
	case SDLK_8:
		return DVL_VK_8;
	case SDLK_9:
		return DVL_VK_9;
	case SDLK_COLON:
		return DVL_VK_OEM_21; // remap?
	case SDLK_SEMICOLON:
		return DVL_VK_OEM_1;
	case SDLK_LESS:
		return DVL_VK_OEM_15; // remap?
	case SDLK_EQUALS:
		return DVL_VK_OEM_PLUS;
	case SDLK_GREATER:
		return DVL_VK_OEM_16; // remap?
	case SDLK_QUESTION:
		return DVL_VK_OEM_COMMA; // remap?
	case SDLK_AT:
		return DVL_VK_OEM_9; // remap?
	case SDLK_LEFTBRACKET:
		return DVL_VK_OEM_4;
	case SDLK_BACKSLASH:
		return DVL_VK_OEM_5;
	case SDLK_RIGHTBRACKET:
		return DVL_VK_OEM_6;
	case SDLK_CARET:
		return DVL_VK_OEM_12; // remap?
	case SDLK_UNDERSCORE:
		return DVL_VK_SEPARATOR; // remap?
	case SDLK_BACKQUOTE:
		return DVL_VK_OEM_3;
	case SDLK_DELETE:
		return DVL_VK_DELETE;
	case SDLK_KP0:
		return DVL_VK_NUMPAD0;
	case SDLK_KP1:
		return DVL_VK_NUMPAD1;
	case SDLK_KP2:
		return DVL_VK_NUMPAD2;
	case SDLK_KP3:
		return DVL_VK_NUMPAD3;
	case SDLK_KP4:
		return DVL_VK_NUMPAD4;
	case SDLK_KP5:
		return DVL_VK_NUMPAD5;
	case SDLK_KP6:
		return DVL_VK_NUMPAD6;
	case SDLK_KP7:
		return DVL_VK_NUMPAD7;
	case SDLK_KP8:
		return DVL_VK_NUMPAD8;
	case SDLK_KP9:
		return DVL_VK_NUMPAD9;
	case SDLK_KP_PERIOD:
		return DVL_VK_DECIMAL;
	case SDLK_KP_DIVIDE:
		return DVL_VK_DIVIDE;
	case SDLK_KP_MULTIPLY:
		return DVL_VK_MULTIPLY;
	case SDLK_KP_MINUS:
		return DVL_VK_SUBTRACT;
	case SDLK_KP_PLUS:
		return DVL_VK_ADD;
	case SDLK_KP_ENTER:
		return DVL_VK_RETURN;
	case SDLK_KP_EQUALS:
		return DVL_VK_OEM_24; // remap?
	case SDLK_UP:
		return DVL_VK_UP;
	case SDLK_DOWN:
		return DVL_VK_DOWN;
	case SDLK_RIGHT:
		return DVL_VK_RIGHT;
	case SDLK_LEFT:
		return DVL_VK_LEFT;
	case SDLK_INSERT:
		return DVL_VK_INSERT;
	case SDLK_HOME:
		return DVL_VK_HOME;
	case SDLK_END:
		return DVL_VK_END;
	case SDLK_PAGEUP:
		return DVL_VK_PRIOR;
	case SDLK_PAGEDOWN:
		return DVL_VK_NEXT;
	case SDLK_NUMLOCK:
		return DVL_VK_NUMLOCK;
	case SDLK_CAPSLOCK:
		return DVL_VK_CAPITAL;
	case SDLK_SCROLLOCK:
		return DVL_VK_SCROLL;
	case SDLK_RSHIFT:
		return DVL_VK_RSHIFT;
	case SDLK_LSHIFT:
		return DVL_VK_LSHIFT;
	case SDLK_RCTRL:
		return DVL_VK_RCONTROL;
	case SDLK_LCTRL:
		return DVL_VK_LCONTROL;
	case SDLK_RALT:
		return DVL_VK_RMENU;
	case SDLK_LALT:
		return DVL_VK_LMENU;
	case SDLK_RMETA:
		return DVL_VK_APPS; // remap?
	case SDLK_LMETA:
		return DVL_VK_EXECUTE; // remap?
	case SDLK_LSUPER:
		return DVL_VK_LWIN;
	case SDLK_RSUPER:
		return DVL_VK_RWIN;
	case SDLK_MODE:
		return DVL_VK_MODECHANGE; // remap?
	case SDLK_HELP:
		return DVL_VK_HELP;
	case SDLK_PRINT:
		return DVL_VK_SNAPSHOT;
	case SDLK_SYSREQ:
		return DVL_VK_OEM_12; // remap?
	case SDLK_BREAK:
		return DVL_VK_OEM_8; // remap?
	case SDLK_MENU:
		return DVL_VK_MENU;
	case SDLK_POWER:
		return DVL_VK_OEM_12; // remap?
	case SDLK_EURO:
		return DVL_VK_OEM_19; // remap?
	default:
		if (ksc <= SDLK_z && ksc >= SDLK_a)
			return DVL_VK_A + (ksc - SDLK_a);
		if (ksc <= SDLK_F15 && ksc >= SDLK_F1)
			return DVL_VK_F1 + (ksc - SDLK_F1);

		app_fatal("Unrecognized key-code %d.", ksc);
		return DVL_VK_NONAME;
	}
#else
	SDL_Scancode ksc = key.scancode;
	switch (ksc) {
	case SDL_SCANCODE_1:
		return DVL_VK_1;
	case SDL_SCANCODE_2:
		return DVL_VK_2;
	case SDL_SCANCODE_3:
		return DVL_VK_3;
	case SDL_SCANCODE_4:
		return DVL_VK_4;
	case SDL_SCANCODE_5:
		return DVL_VK_5;
	case SDL_SCANCODE_6:
		return DVL_VK_6;
	case SDL_SCANCODE_7:
		return DVL_VK_7;
	case SDL_SCANCODE_8:
		return DVL_VK_8;
	case SDL_SCANCODE_9:
		return DVL_VK_9;
	case SDL_SCANCODE_0:
		return DVL_VK_0;
	case SDL_SCANCODE_RETURN:
		return DVL_VK_RETURN;
	case SDL_SCANCODE_ESCAPE:
		return DVL_VK_ESCAPE;
	case SDL_SCANCODE_BACKSPACE:
		return DVL_VK_BACK;
	case SDL_SCANCODE_TAB:
		return DVL_VK_TAB;
	case SDL_SCANCODE_SPACE:
		return DVL_VK_SPACE;
	case SDL_SCANCODE_MINUS:
		return DVL_VK_OEM_MINUS;
	case SDL_SCANCODE_EQUALS:
		return DVL_VK_OEM_PLUS;
	case SDL_SCANCODE_LEFTBRACKET:
		return DVL_VK_OEM_4;
	case SDL_SCANCODE_RIGHTBRACKET:
		return DVL_VK_OEM_6;
	case SDL_SCANCODE_BACKSLASH:
	case SDL_SCANCODE_NONUSHASH:
		return DVL_VK_OEM_5;
	case SDL_SCANCODE_SEMICOLON:
		return DVL_VK_OEM_1;
	case SDL_SCANCODE_APOSTROPHE:
		return DVL_VK_OEM_7;
	case SDL_SCANCODE_GRAVE:
		return DVL_VK_OEM_3;
	case SDL_SCANCODE_COMMA:
		return DVL_VK_OEM_COMMA;
	case SDL_SCANCODE_PERIOD:
		return DVL_VK_OEM_PERIOD;
	case SDL_SCANCODE_SLASH:
		return DVL_VK_OEM_2;
	case SDL_SCANCODE_CAPSLOCK:
		return DVL_VK_CAPITAL;
	case SDL_SCANCODE_PRINTSCREEN:
		return DVL_VK_SNAPSHOT;
	case SDL_SCANCODE_SCROLLLOCK:
		return DVL_VK_SCROLL;
	case SDL_SCANCODE_PAUSE:
		return DVL_VK_PAUSE;
	case SDL_SCANCODE_INSERT:
		return DVL_VK_INSERT;
	case SDL_SCANCODE_HOME:
		return DVL_VK_HOME;
	case SDL_SCANCODE_PAGEUP:
		return DVL_VK_PRIOR;
	case SDL_SCANCODE_DELETE:
		return DVL_VK_DELETE;
	case SDL_SCANCODE_END:
		return DVL_VK_END;
	case SDL_SCANCODE_PAGEDOWN:
		return DVL_VK_NEXT;
	case SDL_SCANCODE_RIGHT:
		return DVL_VK_RIGHT;
	case SDL_SCANCODE_LEFT:
		return DVL_VK_LEFT;
	case SDL_SCANCODE_DOWN:
		return DVL_VK_DOWN;
	case SDL_SCANCODE_UP:
		return DVL_VK_UP;
	case SDL_SCANCODE_NUMLOCKCLEAR:
		return DVL_VK_NUMLOCK;
	case SDL_SCANCODE_KP_DIVIDE:
		return DVL_VK_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY:
		return DVL_VK_MULTIPLY;
	case SDL_SCANCODE_KP_MINUS:
		return DVL_VK_SUBTRACT;
	case SDL_SCANCODE_KP_PLUS:
		return DVL_VK_ADD;
	case SDL_SCANCODE_KP_ENTER:
		return DVL_VK_RETURN;
	case SDL_SCANCODE_KP_1:
		return DVL_VK_NUMPAD1;
	case SDL_SCANCODE_KP_2:
		return DVL_VK_NUMPAD2;
	case SDL_SCANCODE_KP_3:
		return DVL_VK_NUMPAD3;
	case SDL_SCANCODE_KP_4:
		return DVL_VK_NUMPAD4;
	case SDL_SCANCODE_KP_5:
		return DVL_VK_NUMPAD5;
	case SDL_SCANCODE_KP_6:
		return DVL_VK_NUMPAD6;
	case SDL_SCANCODE_KP_7:
		return DVL_VK_NUMPAD7;
	case SDL_SCANCODE_KP_8:
		return DVL_VK_NUMPAD8;
	case SDL_SCANCODE_KP_9:
		return DVL_VK_NUMPAD9;
	case SDL_SCANCODE_KP_0:
		return DVL_VK_NUMPAD0;
	case SDL_SCANCODE_KP_PERIOD:
		return DVL_VK_DECIMAL;
	case SDL_SCANCODE_NONUSBACKSLASH:
		return DVL_VK_OEM_8;
	case SDL_SCANCODE_APPLICATION:
		return DVL_VK_APPS; // remap?
	case SDL_SCANCODE_POWER:
		return DVL_VK_OEM_12; // remap?
	case SDL_SCANCODE_KP_EQUALS:
		return DVL_VK_OEM_13; // remap?
	case SDL_SCANCODE_EXECUTE:
		return DVL_VK_EXECUTE;
	case SDL_SCANCODE_HELP:
		return DVL_VK_HELP;
	case SDL_SCANCODE_MENU:
		return DVL_VK_MENU;
	case SDL_SCANCODE_SELECT:
		return DVL_VK_SELECT;
	case SDL_SCANCODE_STOP:
		return DVL_VK_ATTN; // remap?
	case SDL_SCANCODE_AGAIN:
		return DVL_VK_OEM_12; // remap?
	case SDL_SCANCODE_UNDO:
		return DVL_VK_OEM_13; // remap?
	case SDL_SCANCODE_CUT:
		return DVL_VK_OEM_14; // remap?
	case SDL_SCANCODE_COPY:
		return DVL_VK_OEM_15; // remap?
	case SDL_SCANCODE_PASTE:
		return DVL_VK_OEM_16; // remap?
	case SDL_SCANCODE_FIND:
		return DVL_VK_OEM_17; // remap?
	case SDL_SCANCODE_MUTE:
		return DVL_VK_VOLUME_MUTE;
	case SDL_SCANCODE_VOLUMEUP:
		return DVL_VK_VOLUME_UP;
	case SDL_SCANCODE_VOLUMEDOWN:
		return DVL_VK_VOLUME_DOWN;
	case SDL_SCANCODE_KP_COMMA:
		return DVL_VK_OEM_COMMA;
	case SDL_SCANCODE_KP_EQUALSAS400:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_INTERNATIONAL1:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL2:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL3:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL4:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL5:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL6:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL7:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL8:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_INTERNATIONAL9:
		return DVL_VK_IME_ON; // remap?
	case SDL_SCANCODE_LANG1:
		return DVL_VK_HANGUL; /**< Hangul/English toggle */
	case SDL_SCANCODE_LANG2:
		return DVL_VK_HANJA; /**< Hanja conversion */
	case SDL_SCANCODE_LANG3:
		return DVL_VK_HANGUL; // remap? /**< Katakana */
	case SDL_SCANCODE_LANG4:
		return DVL_VK_JUNJA; // remap? /**< Hiragana */
	case SDL_SCANCODE_LANG5:
		return DVL_VK_FINAL; // remap? /**< Zenkaku/Hankaku */
	case SDL_SCANCODE_LANG6:
		return DVL_VK_IME_OFF; // remap? /**< reserved */
	case SDL_SCANCODE_LANG7:
		return DVL_VK_IME_OFF; // remap? /**< reserved */
	case SDL_SCANCODE_LANG8:
		return DVL_VK_IME_OFF; // remap? /**< reserved */
	case SDL_SCANCODE_LANG9:
		return DVL_VK_IME_OFF; // remap? /**< reserved */
	case SDL_SCANCODE_ALTERASE:
		return DVL_VK_EREOF; // remap?
	case SDL_SCANCODE_SYSREQ:
		return DVL_VK_OEM_12; // remap?
	case SDL_SCANCODE_CANCEL:
		return DVL_VK_CANCEL;
	case SDL_SCANCODE_CLEAR:
		return DVL_VK_OEM_CLEAR;
	case SDL_SCANCODE_PRIOR:
		return DVL_VK_PRIOR;
	case SDL_SCANCODE_RETURN2:
		return DVL_VK_RETURN;
	case SDL_SCANCODE_SEPARATOR:
		return DVL_VK_SEPARATOR;
	case SDL_SCANCODE_OUT:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_OPER:
		return DVL_VK_OEM_23; // remap?
	case SDL_SCANCODE_CLEARAGAIN:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_CRSEL:
		return DVL_VK_CRSEL;
	case SDL_SCANCODE_EXSEL:
		return DVL_VK_EXSEL;
	case SDL_SCANCODE_KP_00:
	case SDL_SCANCODE_KP_000:
		return DVL_VK_NUMPAD0;
	case SDL_SCANCODE_THOUSANDSSEPARATOR:
		return DVL_VK_OEM_17; // remap?
	case SDL_SCANCODE_DECIMALSEPARATOR:
		return DVL_VK_OEM_18; // remap?
	case SDL_SCANCODE_CURRENCYUNIT:
		return DVL_VK_OEM_19; // remap?
	case SDL_SCANCODE_CURRENCYSUBUNIT:
		return DVL_VK_OEM_20; // remap?
	case SDL_SCANCODE_KP_LEFTPAREN:
		return DVL_VK_OEM_21; // remap?
	case SDL_SCANCODE_KP_RIGHTPAREN:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_KP_LEFTBRACE:
		return DVL_VK_OEM_23; // remap?
	case SDL_SCANCODE_KP_RIGHTBRACE:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_KP_TAB:
		return DVL_VK_TAB;
	case SDL_SCANCODE_KP_BACKSPACE:
		return DVL_VK_BACK;
	case SDL_SCANCODE_KP_A:
		return DVL_VK_A;
	case SDL_SCANCODE_KP_B:
		return DVL_VK_B;
	case SDL_SCANCODE_KP_C:
		return DVL_VK_C;
	case SDL_SCANCODE_KP_D:
		return DVL_VK_D;
	case SDL_SCANCODE_KP_E:
		return DVL_VK_E;
	case SDL_SCANCODE_KP_F:
		return DVL_VK_F;
	case SDL_SCANCODE_KP_XOR:
		return DVL_VK_OEM_12; // remap?
	case SDL_SCANCODE_KP_POWER:
		return DVL_VK_OEM_13; // remap?
	case SDL_SCANCODE_KP_PERCENT:
		return DVL_VK_OEM_14; // remap?
	case SDL_SCANCODE_KP_LESS:
		return DVL_VK_OEM_15; // remap?
	case SDL_SCANCODE_KP_GREATER:
		return DVL_VK_OEM_16; // remap?
	case SDL_SCANCODE_KP_AMPERSAND:
		return DVL_VK_OEM_17; // remap?
	case SDL_SCANCODE_KP_DBLAMPERSAND:
		return DVL_VK_OEM_18; // remap?
	case SDL_SCANCODE_KP_VERTICALBAR:
		return DVL_VK_OEM_19; // remap?
	case SDL_SCANCODE_KP_DBLVERTICALBAR:
		return DVL_VK_OEM_20; // remap?
	case SDL_SCANCODE_KP_COLON:
		return DVL_VK_OEM_21; // remap?
	case SDL_SCANCODE_KP_HASH:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_KP_SPACE:
		return DVL_VK_SPACE; // remap?
	case SDL_SCANCODE_KP_AT:
		return DVL_VK_OEM_9; // remap?
	case SDL_SCANCODE_KP_EXCLAM:
		return DVL_VK_OEM_10; // remap?
	case SDL_SCANCODE_KP_MEMSTORE:
		return DVL_VK_OEM_11; // remap?
	case SDL_SCANCODE_KP_MEMRECALL:
		return DVL_VK_OEM_12; // remap?
	case SDL_SCANCODE_KP_MEMCLEAR:
		return DVL_VK_OEM_13; // remap?
	case SDL_SCANCODE_KP_MEMADD:
		return DVL_VK_OEM_14; // remap?
	case SDL_SCANCODE_KP_MEMSUBTRACT:
		return DVL_VK_OEM_15; // remap?
	case SDL_SCANCODE_KP_MEMMULTIPLY:
		return DVL_VK_OEM_16; // remap?
	case SDL_SCANCODE_KP_MEMDIVIDE:
		return DVL_VK_OEM_17; // remap?
	case SDL_SCANCODE_KP_PLUSMINUS:
		return DVL_VK_OEM_18; // remap?
	case SDL_SCANCODE_KP_CLEAR:
		return DVL_VK_OEM_19; // remap?
	case SDL_SCANCODE_KP_CLEARENTRY:
		return DVL_VK_OEM_20; // remap?
	case SDL_SCANCODE_KP_BINARY:
		return DVL_VK_OEM_21; // remap?
	case SDL_SCANCODE_KP_OCTAL:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_KP_DECIMAL:
		return DVL_VK_OEM_23; // remap?
	case SDL_SCANCODE_KP_HEXADECIMAL:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_LCTRL:
		return DVL_VK_LCONTROL;
	case SDL_SCANCODE_LSHIFT:
		return DVL_VK_LSHIFT;
	case SDL_SCANCODE_LALT:
		return DVL_VK_LMENU;
	case SDL_SCANCODE_LGUI:
		return DVL_VK_LWIN;
	case SDL_SCANCODE_RCTRL:
		return DVL_VK_RCONTROL;
	case SDL_SCANCODE_RSHIFT:
		return DVL_VK_RSHIFT;
	case SDL_SCANCODE_RALT:
		return DVL_VK_RMENU;
	case SDL_SCANCODE_RGUI:
		return DVL_VK_RWIN;
	case SDL_SCANCODE_MODE:
		return DVL_VK_MODECHANGE; // remap?
	case SDL_SCANCODE_AUDIONEXT:
		return DVL_VK_MEDIA_NEXT_TRACK;
	case SDL_SCANCODE_AUDIOPREV:
		return DVL_VK_MEDIA_PREV_TRACK;
	case SDL_SCANCODE_AUDIOSTOP:
		return DVL_VK_MEDIA_STOP;
	case SDL_SCANCODE_AUDIOPLAY:
		return DVL_VK_MEDIA_PLAY_PAUSE;
	case SDL_SCANCODE_AUDIOMUTE:
		return DVL_VK_VOLUME_MUTE; // remap?
	case SDL_SCANCODE_MEDIASELECT:
		return DVL_VK_LAUNCH_MEDIA_SELECT;
	case SDL_SCANCODE_WWW:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_MAIL:
		return DVL_VK_LAUNCH_MAIL;
	case SDL_SCANCODE_CALCULATOR:
		return DVL_VK_OEM_23; // remap?
	case SDL_SCANCODE_COMPUTER:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_AC_SEARCH:
		return DVL_VK_BROWSER_SEARCH;
	case SDL_SCANCODE_AC_HOME:
		return DVL_VK_BROWSER_HOME;
	case SDL_SCANCODE_AC_BACK:
		return DVL_VK_BROWSER_BACK;
	case SDL_SCANCODE_AC_FORWARD:
		return DVL_VK_BROWSER_FORWARD;
	case SDL_SCANCODE_AC_STOP:
		return DVL_VK_BROWSER_STOP;
	case SDL_SCANCODE_AC_REFRESH:
		return DVL_VK_BROWSER_REFRESH;
	case SDL_SCANCODE_AC_BOOKMARKS:
		return DVL_VK_BROWSER_FAVORITES;
	case SDL_SCANCODE_BRIGHTNESSDOWN:
		return DVL_VK_OEM_18; // remap?
	case SDL_SCANCODE_BRIGHTNESSUP:
		return DVL_VK_OEM_19; // remap?
	case SDL_SCANCODE_DISPLAYSWITCH:
		return DVL_VK_OEM_20; // remap?
	case SDL_SCANCODE_KBDILLUMTOGGLE:
		return DVL_VK_OEM_21; // remap?
	case SDL_SCANCODE_KBDILLUMDOWN:
		return DVL_VK_OEM_22; // remap?
	case SDL_SCANCODE_KBDILLUMUP:
		return DVL_VK_OEM_23; // remap?
	case SDL_SCANCODE_EJECT:
		return DVL_VK_OEM_24; // remap?
	case SDL_SCANCODE_SLEEP:
		return DVL_VK_SLEEP;
	case SDL_SCANCODE_APP1:
		return DVL_VK_LAUNCH_APP1;
	case SDL_SCANCODE_APP2:
		return DVL_VK_LAUNCH_APP2;
	case SDL_SCANCODE_AUDIOREWIND:
		return DVL_VK_MEDIA_PREV_TRACK;
	case SDL_SCANCODE_AUDIOFASTFORWARD:
		return DVL_VK_MEDIA_NEXT_TRACK;
#if !SDL_VERSION_ATLEAST(2, 24, 0) && (/* __IPHONEOS__ ||*/ __ANDROID__ || __NGAGE__)
	case SDL_SCANCODE_SOFTLEFT:
		return DVL_VK_LEFT;
	case SDL_SCANCODE_SOFTRIGHT:
		return DVL_VK_RIGHT;
	case SDL_SCANCODE_CALL:
		return DVL_VK_RETURN;
	case SDL_SCANCODE_ENDCALL:
		return DVL_VK_ESCAPE;
#endif
	default:
		if (ksc <= SDL_SCANCODE_Z && ksc >= SDL_SCANCODE_A)
			return DVL_VK_A + (ksc - SDL_SCANCODE_A);
		if (ksc <= SDL_SCANCODE_F12 && ksc >= SDL_SCANCODE_F1)
			return DVL_VK_F1 + (ksc - SDL_SCANCODE_F1);
		if (ksc <= SDL_SCANCODE_F24 && ksc >= SDL_SCANCODE_F13)
			return DVL_VK_F13 + (ksc - SDL_SCANCODE_F13);

		ASSUME_UNREACHABLE
		return DVL_VK_NONAME;
	}
#endif // USE_SDL1
	/*
	SDL_Keycode sym = key.sym;
	switch (sym) {
	case SDLK_BACKSPACE:
		return DVL_VK_BACK;
	case SDLK_TAB:
		return DVL_VK_TAB;
	case SDLK_RETURN:
		return DVL_VK_RETURN;
	case SDLK_ESCAPE:
		return DVL_VK_ESCAPE;
	case SDLK_SPACE:
		return DVL_VK_SPACE;
	case SDLK_QUOTE:
		return DVL_VK_OEM_7;
	case SDLK_COMMA:
		return DVL_VK_OEM_COMMA;
	case SDLK_MINUS:
		return DVL_VK_OEM_MINUS;
	case SDLK_PERIOD:
		return DVL_VK_OEM_PERIOD;
	case SDLK_SLASH:
		return DVL_VK_OEM_2;
	case SDLK_SEMICOLON:
		return DVL_VK_OEM_1;
	case SDLK_EQUALS:
		return DVL_VK_OEM_PLUS;
	case SDLK_LEFTBRACKET:
		return DVL_VK_OEM_4;
	case SDLK_BACKSLASH:
		return DVL_VK_OEM_5;
	case SDLK_RIGHTBRACKET:
		return DVL_VK_OEM_6;
	case SDLK_BACKQUOTE:
		return DVL_VK_OEM_3;
	case SDLK_DELETE:
		return DVL_VK_DELETE;
	case SDLK_CAPSLOCK:
		return DVL_VK_CAPITAL;
	case SDLK_PRINTSCREEN:
		return DVL_VK_SNAPSHOT;
	case SDLK_SCROLLLOCK:
		return DVL_VK_SCROLL;
	case SDLK_PAUSE:
		return DVL_VK_PAUSE;
	case SDLK_INSERT:
		return DVL_VK_INSERT;
	case SDLK_HOME:
		return DVL_VK_HOME;
	case SDLK_PAGEUP:
		return DVL_VK_PRIOR;
	case SDLK_END:
		return DVL_VK_END;
	case SDLK_PAGEDOWN:
		return DVL_VK_NEXT;
	case SDLK_RIGHT:
		return DVL_VK_RIGHT;
	case SDLK_LEFT:
		return DVL_VK_LEFT;
	case SDLK_DOWN:
		return DVL_VK_DOWN;
	case SDLK_UP:
		return DVL_VK_UP;
	case SDLK_NUMLOCKCLEAR:
		return DVL_VK_NUMLOCK;
	case SDLK_KP_DIVIDE:
		return DVL_VK_DIVIDE;
	case SDLK_KP_MULTIPLY:
		return DVL_VK_MULTIPLY;
	case SDLK_KP_MINUS:
		return DVL_VK_SUBTRACT;
	case SDLK_KP_PLUS:
		return DVL_VK_ADD;
	case SDLK_KP_ENTER:
		return DVL_VK_RETURN;
	case SDLK_KP_1:
		return DVL_VK_NUMPAD1;
	case SDLK_KP_2:
		return DVL_VK_NUMPAD2;
	case SDLK_KP_3:
		return DVL_VK_NUMPAD3;
	case SDLK_KP_4:
		return DVL_VK_NUMPAD4;
	case SDLK_KP_5:
		return DVL_VK_NUMPAD5;
	case SDLK_KP_6:
		return DVL_VK_NUMPAD6;
	case SDLK_KP_7:
		return DVL_VK_NUMPAD7;
	case SDLK_KP_8:
		return DVL_VK_NUMPAD8;
	case SDLK_KP_9:
		return DVL_VK_NUMPAD9;
	case SDLK_KP_0:
		return DVL_VK_NUMPAD0;
	case SDLK_KP_PERIOD:
		return DVL_VK_DECIMAL;
	case SDLK_MENU:
		return DVL_VK_MENU;
#ifndef USE_SDL1
	case SDLK_KP_COMMA:
		return DVL_VK_OEM_COMMA;
#endif
	case SDLK_LCTRL:
		return DVL_VK_LCONTROL;
	case SDLK_LSHIFT:
		return DVL_VK_LSHIFT;
	case SDLK_LALT:
		return DVL_VK_LMENU;
	case SDLK_LGUI:
		return DVL_VK_LWIN;
	case SDLK_RCTRL:
		return DVL_VK_RCONTROL;
	case SDLK_RSHIFT:
		return DVL_VK_RSHIFT;
	case SDLK_RALT:
		return DVL_VK_RMENU;
	case SDLK_RGUI:
		return DVL_VK_RWIN;
	default:
		if (sym >= SDLK_a && sym <= SDLK_z)
			return 'A' + (sym - SDLK_a);
		if (sym >= SDLK_0 && sym <= SDLK_9)
			return '0' + (sym - SDLK_0);
		if (sym >= SDLK_F1 && sym <= SDLK_F12)
			return DVL_VK_F1 + (sym - SDLK_F1);
#if DEBUG_MODE
		DoLog("unknown key: name=%s sym=0x%X scan=%d mod=0x%X", SDL_GetKeyName(sym), sym, key.scancode, key.mod);
#endif
		return sym;
	}*/
}

/*static WPARAM PositionForMouse(Sint32 x, Sint32 y)
{
	return (((uint16_t)(y & 0xFFFF)) << 16) | (uint16_t)(x & 0xFFFF);
}

static WPARAM KeystateForMouse(WPARAM ret)
{
	ret |= (SDL_GetModState() & KMOD_SHIFT) ? DVL_MK_SHIFT : 0;
	// XXX: other DVL_MK_* codes not implemented
	return ret;
}*/

#if DEBUG_MODE
static bool FalseAvail(const char* name, int value)
{
	DoLog("Unhandled SDL event: %s %d", name, value);
	return true;
}
#endif

bool PeekMessage(Dvl_Event &e)
{
#ifdef __SWITCH__
	HandleDocking();
#endif

	if (!SDL_PollEvent(&e)) {
		return false;
	}


#if HAS_TOUCHPAD
	handle_touch(&e);
#endif

	Uint32 type = e.type;
	e.type = DVL_WM_NONE;

#ifdef USE_SDL1
	if (type == SDL_MOUSEMOTION) {
		OutputToLogical(&e.motion.x, &e.motion.y);
	} else if (type == SDL_MOUSEBUTTONDOWN || type == SDL_MOUSEBUTTONUP) {
		OutputToLogical(&e.button.x, &e.button.y);
	}
	if ((type == SDL_KEYUP || type == SDL_KEYDOWN) && e.key.keysym.sym == SDLK_UNKNOWN) {
		// Erroneous events generated by RG350 kernel. (Assumes RG350 to use SDL1)
		return true;
	}
#endif

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#if (HAS_TOUCHPAD || HAS_DPAD) && !defined(USE_SDL1)
	if ((type >= SDL_KEYDOWN && type < SDL_JOYAXISMOTION) || (type >= SDL_FINGERDOWN && type < SDL_DOLLARGESTURE)) {
#else
	if (type >= SDL_KEYDOWN && type < SDL_JOYAXISMOTION) {
#endif
		// Keyboard or Mouse (or Touch) events -> switch to standard input
#if FIX_WARPING
		if (!mouseWarping || type != SDL_MOUSEMOTION)
#endif
			sgbControllerActive = false;
	}
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

	switch (type) {
	case SDL_QUIT:
		e.type = DVL_WM_QUIT;
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		int key = TranslateSdlKey(e.key.keysym);
		//if (key == -1)
		//	return FalseAvail(type == SDL_KEYDOWN ? "SDL_KEYDOWN" : "SDL_KEYUP", e.key.keysym.sym);
		e.type = type == SDL_KEYDOWN ? DVL_WM_KEYDOWN : DVL_WM_KEYUP;
		e.vkcode = key;
	} break;
	case SDL_MOUSEMOTION:
#if FIX_WARPING
		mouseWarping = false;
#endif
		e.type = DVL_WM_MOUSEMOVE;
		MousePos.x = e.motion.x;
		MousePos.y = e.motion.y;
		break;
	case SDL_MOUSEBUTTONDOWN: {
		int button = e.button.button;
		if (button == SDL_BUTTON_LEFT) {
			e.type = DVL_WM_LBUTTONDOWN;
			//lpMsg->wParam = PositionForMouse(e.button.x, e.button.y); -- BUTTON_POSITION
			//lpMsg->lParam = KeystateForMouse(DVL_MK_LBUTTON); -- unused
		} else if (button == SDL_BUTTON_RIGHT) {
			e.type = DVL_WM_RBUTTONDOWN;
			//lpMsg->wParam = PositionForMouse(e.button.x, e.button.y); -- BUTTON_POSITION
			//lpMsg->lParam = KeystateForMouse(DVL_MK_RBUTTON); -- unused
		} else if (button == SDL_BUTTON_MIDDLE) {
			e.type = DVL_WM_KEYDOWN;
			e.vkcode = DVL_VK_MBUTTON;
		} else if (button == SDL_BUTTON_X1) {
			e.type = DVL_WM_KEYDOWN;
			e.vkcode = DVL_VK_XBUTTON1;
#ifdef USE_SDL1
		} else if (button == SDL_BUTTON_WHEELUP) {
			e.type = DVL_WM_KEYDOWN;
			e.vkcode = (SDL_GetModState() & KMOD_CTRL) ? DVL_VK_OEM_PLUS : DVL_VK_UP;
		} else if (button == SDL_BUTTON_WHEELDOWN) {
			e.type = DVL_WM_KEYDOWN;
			e.vkcode = (SDL_GetModState() & KMOD_CTRL) ? DVL_VK_OEM_MINUS : DVL_VK_DOWN;
#endif
		}
	} break;
	case SDL_MOUSEBUTTONUP: {
		int button = e.button.button;
		if (button == SDL_BUTTON_LEFT) {
			e.type = DVL_WM_LBUTTONUP;
			//lpMsg->wParam = PositionForMouse(e.button.x, e.button.y); -- BUTTON_POSITION
			//lpMsg->lParam = KeystateForMouse(0); -- unused
		} else if (button == SDL_BUTTON_RIGHT) {
			e.type = DVL_WM_RBUTTONUP;
			//lpMsg->wParam = PositionForMouse(e.button.x, e.button.y); -- BUTTON_POSITION
			//lpMsg->lParam = KeystateForMouse(0); -- unused
		} else if (button == SDL_BUTTON_MIDDLE) {
			e.type = DVL_WM_KEYUP;
			e.vkcode = DVL_VK_MBUTTON;
		} else if (button == SDL_BUTTON_X1) {
			e.type = DVL_WM_KEYUP;
			e.vkcode = DVL_VK_XBUTTON1;
		}
	} break;
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL: {
		e.type = DVL_WM_KEYDOWN;
		int key;
		unsigned mod = SDL_GetModState();
		if (e.wheel.y > 0) {
			key = (mod & KMOD_CTRL) ? DVL_VK_OEM_PLUS : DVL_VK_UP;
		} else if (e.wheel.y < 0) {
			key = (mod & KMOD_CTRL) ? DVL_VK_OEM_MINUS : DVL_VK_DOWN;
		} else {
			key = e.wheel.x >= 0 ? DVL_VK_LEFT : DVL_VK_RIGHT;
		}
		e.vkcode = key;
		e.key.keysym.mod = mod;
	} break;
#if HAS_GAMECTRL
	case SDL_CONTROLLERDEVICEADDED:
		GameController::Add(e.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		GameController::Remove(e.cdevice.which);
		break;
#endif
#if HAS_JOYSTICK
	case SDL_JOYDEVICEADDED:
		Joystick::Add(e.jdevice.which);
		break;
	case SDL_JOYDEVICEREMOVED:
		Joystick::Remove(e.jdevice.which);
		break;
#endif
#if DEBUG_MODE
#if SDL_VERSION_ATLEAST(2, 0, 4)
	case SDL_AUDIODEVICEADDED:
		return FalseAvail("SDL_AUDIODEVICEADDED", e.adevice.which);
	case SDL_AUDIODEVICEREMOVED:
		return FalseAvail("SDL_AUDIODEVICEREMOVED", e.adevice.which);
	case SDL_KEYMAPCHANGED:
		return FalseAvail("SDL_KEYMAPCHANGED", 0);
#endif // SDL_VERSION_ATLEAST(2, 0, 4)
	case SDL_TEXTEDITING:
		return FalseAvail("SDL_TEXTEDITING", e.edit.length);
#endif // DEBUG_MODE
	case SDL_TEXTINPUT:
		e.type = DVL_WM_TEXT;
		break;
	case SDL_WINDOWEVENT:
		switch (e.window.event) {
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
		case SDL_WINDOWEVENT_RESTORED:
			gbWndActive = true;
			e.type = DVL_WM_PAINT;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_WINDOWEVENT_MINIMIZED:
			gbWndActive = false;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			e.type = DVL_WM_CAPTURECHANGED;
			break;
		case SDL_WINDOWEVENT_MOVED:
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_FOCUS_LOST:
#if SDL_VERSION_ATLEAST(2, 0, 5)
		case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
			break;
		case SDL_WINDOWEVENT_ENTER:
#if FIX_WARPING
			// Bug in SDL, SDL_WarpMouseInWindow doesn't emit SDL_MOUSEMOTION
			// and SDL_GetMouseState gives previous location if mouse was
			// outside window (observed on Ubuntu 19.04)
			if (mouseWarping) {
				MousePos = mouseWarpingPos;
				mouseWarping = false;
			}
#endif
			break;
		case SDL_WINDOWEVENT_CLOSE:
			// -- no need to handle, wait for the QUIT event
			// e.type = DVL_WM_QUERYENDSESSION;
			break;
#if DEBUG_MODE
		default:
			return FalseAvail("SDL_WINDOWEVENT", e.window.event);
#endif // DEBUG_MODE
		}

		break;
#endif // !USE_SDL1
	case SDL_USEREVENT:
		// lpMsg->wParam = e.user.data1;
		e.type = e.user.code;
		break;
	default: {
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		e.type = type;
		const ControllerButtonEvent ctrlEvent = ToControllerButtonEvent(e);
		if (ProcessControllerMotion(e)) {
			e.type = DVL_WM_NONE;
			sgbControllerActive = true;
			break;
		}
		e.type = DVL_WM_NONE;
#if HAS_DPAD
		if (!dpad_hotkeys && SimulateRightStickWithDpad(ctrlEvent)) {
			sgbControllerActive = true;
			break;
		}
#endif
		GameAction action = GameAction(GameActionType_NONE);
		if (GetGameAction(ctrlEvent, &action)) {
			if (action.type == GameActionType_SEND_KEY) {
				sgbControllerActive = true;
				e.type = action.send_key.up ? DVL_WM_KEYUP : DVL_WM_KEYDOWN;
				e.vkcode = action.send_key.vk_code;
				e.key.keysym.mod = SDL_GetModState();
			} else if (action.type == GameActionType_SEND_MOUSE_CLICK) {
				sgbControllerActive = false;
				if (action.send_mouse_click.button == GameActionSendMouseClick::LEFT) {
					e.type = action.send_mouse_click.up ? DVL_WM_LBUTTONUP : DVL_WM_LBUTTONDOWN;
				} else {
					e.type = action.send_mouse_click.up ? DVL_WM_RBUTTONUP : DVL_WM_RBUTTONDOWN;
				}
				//lpMsg->wParam = PositionForMouse(MousePos.x, MousePos.y); -- BUTTON_POSITION: assume correct order of events (1: MOTION, 2: button down, [3: MOTION], 4: up)
			}
			break;
		}
#endif
#if DEBUG_MODE
		return FalseAvail("unknown", type);
#endif // DEBUG_MODE
	} break;
	}
	return true;
}

#if defined(USE_SDL1) || DEBUG_MODE
/*
 * Translate keys of keydown events to chars.
 *
 * 'Translation' is necessary in case of SDL1 and to process debug messages.
 * In case of SDL2, the input text is handled using
 * SDL_StartTextInput/SDL_StopTextInput in case 'gbTalkflag' is set.
 * @param key: the key which was pressed (DVL_VK_*)
 * @return the char value of the key, or zero if it can not be translated
 */
int TranslateKey2Char(int key)
{
	//unsigned mod = lpMsg->lParam >> 16;
	SDL_Keymod mod = SDL_GetModState();

	bool shift = (mod & KMOD_SHIFT) != 0;
	if (key >= DVL_VK_A && key <= DVL_VK_Z) {
		static_assert(DVL_VK_A == 'A', "Translation from DVL_VK_A-Z to A-Z is a NOP in TranslateKey2Char I.");
		static_assert(DVL_VK_Z == 'Z', "Translation from DVL_VK_A-Z to A-Z is a NOP in TranslateKey2Char II.");
		if (shift == ((mod & KMOD_CAPS) != 0))
			key = tolower(key);
	} else if (key >= DVL_VK_0 && key <= DVL_VK_9) {
		static_assert(DVL_VK_0 == '0', "Translation from DVL_VK_0-9 to 0-9 is a NOP in TranslateKey2Char I.");
		static_assert(DVL_VK_9 == '9', "Translation from DVL_VK_0-9 to 0-9 is a NOP in TranslateKey2Char II.");
		if (shift) {
			const char shkeys[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
			key = shkeys[key - '0'];
		}
	} else if (key == DVL_VK_SPACE) { // || key == DVL_VK_BACK || key == DVL_VK_ESCAPE || key == DVL_VK_TAB || key == DVL_VK_RETURN) {
		// control keys
		key = ' ';
	} else if (key >= DVL_VK_NUMPAD0 && key <= DVL_VK_DIVIDE) {
		// numpad buttons
		switch (key) {
		case DVL_VK_MULTIPLY:
			key = '*';
			break;
		case DVL_VK_ADD:
			key = '+';
			break;
		case DVL_VK_SUBTRACT:
			key = '-';
			break;
		case DVL_VK_DECIMAL:
			key = ',';
			break;
		case DVL_VK_DIVIDE:
			key = '/';
			break;
		default:
			key = '0' + (key - DVL_VK_NUMPAD0);
			break;
		}
	} else if (key >= DVL_VK_OEM_1 && key <= 0xFF) {
		// oem keys - This probably only supports US keyboard layout
		switch (key) {
		case DVL_VK_OEM_1:
			key = shift ? ':' : ';';
			break;
		case DVL_VK_OEM_2:
			key = shift ? '?' : '/';
			break;
		case DVL_VK_OEM_3:
			key = shift ? '~' : '`';
			break;
		case DVL_VK_OEM_4:
			key = shift ? '{' : '[';
			break;
		case DVL_VK_OEM_5:
			key = shift ? '|' : '\\';
			break;
		case DVL_VK_OEM_6:
			key = shift ? '}' : ']';
			break;
		case DVL_VK_OEM_7:
			key = shift ? '"' : '\'';
			break;
		case DVL_VK_OEM_MINUS:
			key = shift ? '_' : '-';
			break;
		case DVL_VK_OEM_PLUS:
			key = shift ? '+' : '=';
			break;
		case DVL_VK_OEM_PERIOD:
			key = shift ? '>' : '.';
			break;
		case DVL_VK_OEM_COMMA:
			key = shift ? '<' : ',';
			break;
		default:
			key = '?'; // UNIMPLEMENTED();
		}
	} else
		return 0;

	if (key >= ' ') {
		DoLog("char: %c", key);
	}
	return key;
}
#endif

void DispatchMessage(const Dvl_Event* event)
{
	assert(CurrentWndProc != NULL);

	CurrentWndProc(event);
}

void PostMessage(UINT type /*, WPARAM wParam*/)
{
	SDL_Event e;

	e.user.type = SDL_USEREVENT;
	e.user.code = type;
	//e.user.data1 = (void*)wParam;

	SDL_PushEvent(&e);
}

/*void MainWndProc(UINT Msg)
{
	switch (Msg) {
	case DVL_WM_PAINT:
		// gbRedrawFlags |= REDRAW_DRAW_ALL;
		break;
	//case DVL_WM_QUERYENDSESSION:
	//	diablo_quit(EX_OK);
	//	break;
	}
}*/

WNDPROC SetWindowProc(WNDPROC newWndProc)
{
	WNDPROC oldWndProc;

	oldWndProc = CurrentWndProc;
	CurrentWndProc = newWndProc;
	return oldWndProc;
}

DEVILUTION_END_NAMESPACE
