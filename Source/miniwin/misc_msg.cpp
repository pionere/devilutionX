#include <SDL.h>
#include <cstdint>
#include <deque>
#include "utils/utf8.h"

#include "all.h"
#include "controls/controller.h"
#include "controls/controller_motion.h"
#include "controls/game_controls.h"
#include "controls/plrctrls.h"
#include "controls/remap_keyboard.h"
#include "controls/touch.h"
#include "utils/display.h"
#include "utils/sdl_compat.h"
#include "utils/stubs.h"

#ifdef __SWITCH__
#include "platform/switch/docking.h"
#include <switch.h>
#endif

/** @file
 * *
 * Windows message handling and keyboard event conversion for SDL.
 */

DEVILUTION_BEGIN_NAMESPACE

static std::deque<MSG> message_queue;

bool mouseWarping = false;
int mouseWarpingX;
int mouseWarpingY;

void SetCursorPos(int x, int y)
{
	mouseWarpingX = x;
	mouseWarpingY = y;
	mouseWarping = true;
	LogicalToOutput(&x, &y);
	SDL_WarpMouseInWindow(ghMainWnd, x, y);
}

// Moves the mouse to the first attribute "+" button.
void FocusOnCharInfo()
{
	if (gbInvflag || players[myplr]._pStatPts <= 0)
		return;

	// Jump to the first incrementable stat.
	const RECT32 &rect = ChrBtnsRect[0];
	SetCursorPos(rect.x + (rect.w / 2), rect.y + (rect.h / 2));
}

static int TranslateSdlKey(SDL_Keysym key)
{
	// ref: https://wiki.libsdl.org/SDL_Keycode
	// ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
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
		return DVL_VK_ATTN;	// remap?
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
	case SDL_SCANCODE_LANG1: /**< Hangul/English toggle */
		return DVL_VK_HANGUL;
	case SDL_SCANCODE_LANG2: /**< Hanja conversion */
		return DVL_VK_HANJA;
	case SDL_SCANCODE_LANG3: /**< Katakana */
		return DVL_VK_HANGUL; // remap?
	case SDL_SCANCODE_LANG4: /**< Hiragana */
		return DVL_VK_JUNJA; // remap?
	case SDL_SCANCODE_LANG5: /**< Zenkaku/Hankaku */
		return DVL_VK_FINAL; // remap?
	case SDL_SCANCODE_LANG6: /**< reserved */
		return DVL_VK_IME_OFF; // remap?
	case SDL_SCANCODE_LANG7: /**< reserved */
		return DVL_VK_IME_OFF; // remap?
	case SDL_SCANCODE_LANG8: /**< reserved */
		return DVL_VK_IME_OFF; // remap?
	case SDL_SCANCODE_LANG9: /**< reserved */
		return DVL_VK_IME_OFF; // remap?
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
#ifdef _DEBUG
		SDL_Log("unknown key: name=%s sym=0x%X scan=%d mod=0x%X", SDL_GetKeyName(sym), sym, key.scancode, key.mod);
#endif
		return sym;
	}*/
}

static LPARAM PositionForMouse(short x, short y)
{
	return (((uint16_t)(y & 0xFFFF)) << 16) | (uint16_t)(x & 0xFFFF);
}

static WPARAM KeystateForMouse(WPARAM ret)
{
	ret |= (SDL_GetModState() & KMOD_SHIFT) ? DVL_MK_SHIFT : 0;
	// XXX: other DVL_MK_* codes not implemented
	return ret;
}

static bool FalseAvail(const char *name, int value)
{
	//SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Unhandled SDL event: %s %d", name, value);
	SDL_Log("Unhandled SDL event: %s %d", name, value);
	return true;
}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
/**
 * @brief Try to clean the inventory related cursor states.
 * @return True if it is safe to close the inventory
 */
bool BlurInventory()
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		if (!TryDropItem()) {
			int pc = players[myplr]._pClass;
			PlaySFX(sgSFXSets[SFXS_PLR_16][pc], sgSFXSets[SFXS_PLR_16][pc] == PS_WARR16 ? 3 : 1);
			return false;
		}
	}

	gbInvflag = false;
	if (pcurs > CURSOR_HAND)
		NewCursor(CURSOR_HAND);
	if (gbChrflag)
		FocusOnCharInfo();

	return true;
}
#endif

bool PeekMessage(LPMSG lpMsg)
{
#ifdef __SWITCH__
	HandleDocking();
#endif

	if (!message_queue.empty()) {
		*lpMsg = message_queue.front();
		message_queue.pop_front();
		return true;
	}

	SDL_Event e;
	if (!SDL_PollEvent(&e)) {
		return false;
	}

	lpMsg->message = 0;
	lpMsg->lParam = 0;
	lpMsg->wParam = 0;

	if (e.type == SDL_QUIT) {
		lpMsg->message = DVL_WM_QUIT;
		return true;
	}

#if HAS_TOUCHPAD == 1
	handle_touch(&e, MouseX, MouseY);
#endif

#ifdef USE_SDL1
	if (e.type == SDL_MOUSEMOTION) {
		OutputToLogical(&e.motion.x, &e.motion.y);
	} else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
		OutputToLogical(&e.button.x, &e.button.y);
	}
	if ((e.type == SDL_KEYUP || e.type == SDL_KEYDOWN) && e.key.keysym.sym == SDLK_UNKNOWN) {
		// Erroneous events generated by RG350 kernel. (Assumes RG350 to use SDL1)
		return true;
	}
#endif

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (HandleControllerAddedOrRemovedEvent(e))
		return true;

	const ControllerButtonEvent ctrlEvent = ToControllerButtonEvent(e);
	if (ProcessControllerMotion(e, ctrlEvent))
		return true;

	GameAction action;
	if (GetGameAction(e, ctrlEvent, &action)) {
		if (action.type != GameActionType_NONE) {
			sgbControllerActive = true;

			if (gbMoviePlaying) {
				lpMsg->message = DVL_WM_KEYDOWN;
				if (action.type == GameActionType_SEND_KEY)
					lpMsg->wParam = action.send_key.vk_code;
				return true;
			}
		}

		switch (action.type) {
		case GameActionType_NONE:
			break;
		case GameActionType_USE_HEALTH_POTION:
			UseBeltItem(BLT_HEALING);
			break;
		case GameActionType_USE_MANA_POTION:
			UseBeltItem(BLT_MANA);
			break;
		case GameActionType_PRIMARY_ACTION:
			PerformPrimaryAction();
			break;
		case GameActionType_SECONDARY_ACTION:
			PerformSecondaryAction();
			break;
		case GameActionType_CAST_SPELL:
			PerformSpellAction();
			break;
		case GameActionType_TOGGLE_QUICK_SPELL_MENU:
			if (!gbInvflag || BlurInventory()) {
				if (!gbSkillListFlag)
					DoSkillList(true);
				else
					gbSkillListFlag = false;
				gbChrflag = false;
				gbQuestlog = false;
				gbSbookflag = false;
				gbTeamFlag = false;
				StoreSpellCoords();
			}
			break;
		case GameActionType_TOGGLE_CHARACTER_INFO:
			gbChrflag = !gbChrflag;
			if (gbChrflag) {
				gbQuestlog = false;
				gbSkillListFlag = false;
				if (pcurs == CURSOR_DISARM)
					NewCursor(CURSOR_HAND);
				FocusOnCharInfo();
			}
			break;
		case GameActionType_TOGGLE_QUEST_LOG:
			if (!gbQuestlog) {
				StartQuestlog();
				gbChrflag = false;
				gbSkillListFlag = false;
			} else {
				gbQuestlog = false;
			}
			break;
		case GameActionType_TOGGLE_INVENTORY:
			if (gbInvflag) {
				BlurInventory();
			} else {
				gbSbookflag = false;
				gbSkillListFlag = false;
				gbTeamFlag = false;
				gbInvflag = true;
				if (pcurs == CURSOR_DISARM)
					NewCursor(CURSOR_HAND);
				FocusOnInventory();
			}
			break;
		case GameActionType_TOGGLE_SPELL_BOOK:
			if (BlurInventory()) {
				gbInvflag = false;
				gbSkillListFlag = false;
				gbTeamFlag = false;
				gbSbookflag = !gbSbookflag;
			}
			break;
		case GameActionType_SEND_KEY:
			lpMsg->message = action.send_key.up ? DVL_WM_KEYUP : DVL_WM_KEYDOWN;
			lpMsg->wParam = action.send_key.vk_code;
			return true;
		case GameActionType_SEND_MOUSE_CLICK:
			sgbControllerActive = false;
			switch (action.send_mouse_click.button) {
			case GameActionSendMouseClick::LEFT:
				lpMsg->message = action.send_mouse_click.up ? DVL_WM_LBUTTONUP : DVL_WM_LBUTTONDOWN;
				break;
			case GameActionSendMouseClick::RIGHT:
				lpMsg->message = action.send_mouse_click.up ? DVL_WM_RBUTTONUP : DVL_WM_RBUTTONDOWN;
				break;
			}
			lpMsg->lParam = PositionForMouse(MouseX, MouseY);
			break;
		}
		return true;
	}
#endif
#if (HAS_TOUCHPAD == 1 || HAS_DPAD == 1) && !defined(USE_SDL1)
	if (e.type < SDL_JOYAXISMOTION || (e.type >= SDL_FINGERDOWN && e.type < SDL_DOLLARGESTURE)) {
#else
	if (e.type < SDL_JOYAXISMOTION) {
#endif
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
		if (!mouseWarping || e.type != SDL_MOUSEMOTION)
			sgbControllerActive = false;
#endif
		if (mouseWarping && e.type == SDL_MOUSEMOTION)
			mouseWarping = false;
	}

	switch (e.type) {
	case SDL_QUIT:
		lpMsg->message = DVL_WM_QUIT;
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		int key = TranslateSdlKey(e.key.keysym);
		//if (key == -1)
		//	return FalseAvail(e.type == SDL_KEYDOWN ? "SDL_KEYDOWN" : "SDL_KEYUP", e.key.keysym.sym);
		lpMsg->message = e.type == SDL_KEYDOWN ? DVL_WM_KEYDOWN : DVL_WM_KEYUP;
		lpMsg->wParam = (DWORD)key;
#ifdef _DEBUG
		// HACK: Encode modifier in lParam for TranslateMessage later
		lpMsg->lParam = e.key.keysym.mod << 16;
#endif
	} break;
	case SDL_MOUSEMOTION:
		lpMsg->message = DVL_WM_MOUSEMOVE;
		lpMsg->lParam = PositionForMouse(e.motion.x, e.motion.y);
		lpMsg->wParam = KeystateForMouse(0);
		break;
	case SDL_MOUSEBUTTONDOWN: {
		int button = e.button.button;
		if (button == SDL_BUTTON_LEFT) {
			lpMsg->message = DVL_WM_LBUTTONDOWN;
			lpMsg->lParam = PositionForMouse(e.button.x, e.button.y);
			lpMsg->wParam = KeystateForMouse(DVL_MK_LBUTTON);
		} else if (button == SDL_BUTTON_RIGHT) {
			lpMsg->message = DVL_WM_RBUTTONDOWN;
			lpMsg->lParam = PositionForMouse(e.button.x, e.button.y);
			lpMsg->wParam = KeystateForMouse(DVL_MK_RBUTTON);
		}
	} break;
	case SDL_MOUSEBUTTONUP: {
		int button = e.button.button;
		if (button == SDL_BUTTON_LEFT) {
			lpMsg->message = DVL_WM_LBUTTONUP;
			lpMsg->lParam = PositionForMouse(e.button.x, e.button.y);
			lpMsg->wParam = KeystateForMouse(0);
		} else if (button == SDL_BUTTON_RIGHT) {
			lpMsg->message = DVL_WM_RBUTTONUP;
			lpMsg->lParam = PositionForMouse(e.button.x, e.button.y);
			lpMsg->wParam = KeystateForMouse(0);
		}
	} break;
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL:
		lpMsg->message = DVL_WM_KEYDOWN;
		if (e.wheel.y > 0) {
			lpMsg->wParam = GetAsyncKeyState(DVL_VK_CONTROL) ? DVL_VK_OEM_PLUS : DVL_VK_UP;
		} else if (e.wheel.y < 0) {
			lpMsg->wParam = GetAsyncKeyState(DVL_VK_CONTROL) ? DVL_VK_OEM_MINUS : DVL_VK_DOWN;
		} else if (e.wheel.x > 0) {
			lpMsg->wParam = DVL_VK_LEFT;
		} else if (e.wheel.x < 0) {
			lpMsg->wParam = DVL_VK_RIGHT;
		}
		break;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	case SDL_AUDIODEVICEADDED:
		return FalseAvail("SDL_AUDIODEVICEADDED", e.adevice.which);
	case SDL_AUDIODEVICEREMOVED:
		return FalseAvail("SDL_AUDIODEVICEREMOVED", e.adevice.which);
	case SDL_KEYMAPCHANGED:
		return FalseAvail("SDL_KEYMAPCHANGED", 0);
#endif
	case SDL_TEXTEDITING:
		return FalseAvail("SDL_TEXTEDITING", e.edit.length);
	case SDL_TEXTINPUT:
		lpMsg->message = DVL_WM_CHAR;
		lpMsg->wParam = utf8_to_latin1(e.text.text).c_str()[0];
		break;
	case SDL_WINDOWEVENT:
		switch (e.window.event) {
		case SDL_WINDOWEVENT_SHOWN:
			gbActive = true;
			lpMsg->message = DVL_WM_PAINT;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			gbActive = false;
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			lpMsg->message = DVL_WM_PAINT;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			lpMsg->message = DVL_WM_CAPTURECHANGED;
			break;
		case SDL_WINDOWEVENT_MOVED:
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MINIMIZED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_FOCUS_LOST:
#if SDL_VERSION_ATLEAST(2, 0, 5)
		case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
			break;
		case SDL_WINDOWEVENT_ENTER:
			// Bug in SDL, SDL_WarpMouseInWindow doesn't emit SDL_MOUSEMOTION
			// and SDL_GetMouseState gives previous location if mouse was
			// outside window (observed on Ubuntu 19.04)
			if (mouseWarping) {
				MouseX = mouseWarpingX;
				MouseY = mouseWarpingY;
				mouseWarping = false;
			}
			break;
		case SDL_WINDOWEVENT_CLOSE:
			lpMsg->message = DVL_WM_QUERYENDSESSION;
			break;
		default:
			return FalseAvail("SDL_WINDOWEVENT", e.window.event);
		}

		break;
#endif
	default:
		return FalseAvail("unknown", e.type);
	}
	return true;
}

/*
 * Translate keydown events to char events only if gbTalkflag is set (or in debug mode).
 *
 * 'Translation' is no longer necessary. The input text is handled using
 * SDL_StartTextInput/SDL_StopTextInput in case 'gbTalkflag' is set.
 *
 * Remark: HACK in PeekMessage needs to be re-enabled in case TranslateMessage is used
 *  in a non-debug environment.
 */
bool TranslateMessage(const MSG *lpMsg)
{
#ifdef _DEBUG
	if (lpMsg->message == DVL_WM_KEYDOWN) {
		int key = lpMsg->wParam;
		unsigned mod = (DWORD)lpMsg->lParam >> 16;

		bool shift = (mod & KMOD_SHIFT) != 0;
		if (key >= 'A' && key <= 'Z') {
			if (shift == ((mod & KMOD_CAPS) != 0))
				key = tolower(key);
		} else if (key >= '0' && key <= '9') {
			if (shift) {
				const char shkeys[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
				key = shkeys[key - '0'];
			}
		} else if (key == DVL_VK_SPACE) {// || key == DVL_VK_BACK || key == DVL_VK_ESCAPE || key == DVL_VK_TAB || key == DVL_VK_RETURN) {
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
			return true;

#ifdef _DEBUG
		if (key >= 32) {
			SDL_Log("char: %c", key);
		}
#endif
		// XXX: This does not add extended info to lParam
		PostMessage(DVL_WM_CHAR, key, 0);
	}

#endif
	return true;
}

BYTE GetAsyncKeyState(int vKey)
{
	/*if (vKey == DVL_MK_LBUTTON)
		return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
	if (vKey == DVL_MK_RBUTTON)
		return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
	const Uint8 *state = SDLC_GetKeyState();
	switch (vKey) {
	case DVL_VK_CONTROL:
		return (state[SDLC_KEYSTATE(LCTRL)] || state[SDLC_KEYSTATE(RCTRL)]) ? 0x8000 : 0;
	case DVL_VK_SHIFT:
		return (state[SDLC_KEYSTATE(LSHIFT)] || state[SDLC_KEYSTATE(RSHIFT)]) ? 0x8000 : 0;
	case DVL_VK_MENU:
		return (state[SDLC_KEYSTATE(LALT)] || state[SDLC_KEYSTATE(RALT)]) ? 0x8000 : 0;
	case DVL_VK_LEFT:
		return state[SDLC_KEYSTATE(LEFT)] ? 0x8000 : 0;
	case DVL_VK_UP:
		return state[SDLC_KEYSTATE(UP)] ? 0x8000 : 0;
	case DVL_VK_RIGHT:
		return state[SDLC_KEYSTATE(RIGHT)] ? 0x8000 : 0;
	case DVL_VK_DOWN:
		return state[SDLC_KEYSTATE(DOWN)] ? 0x8000 : 0;
	default:
		return 0;
	}*/
	const Uint8 *state = SDLC_GetKeyState();
	if (vKey == DVL_VK_SHIFT) {
		return state[SDLC_KEYSTATE(LSHIFT)] | state[SDLC_KEYSTATE(RSHIFT)];
	}
	if (vKey == DVL_VK_MENU) {
		return state[SDLC_KEYSTATE(LALT)] | state[SDLC_KEYSTATE(RALT)];
	}
	assert(vKey == DVL_VK_CONTROL);
	return state[SDLC_KEYSTATE(LCTRL)] | state[SDLC_KEYSTATE(RCTRL)];
}

void DispatchMessage(const MSG *lpMsg)
{
	assert(CurrentWndProc != NULL);

	CurrentWndProc(lpMsg->message, lpMsg->wParam, lpMsg->lParam);
}

bool PostMessage(UINT type, WPARAM wParam, LPARAM lParam)
{
	MSG message;
	message.message = type;
	message.wParam = wParam;
	message.lParam = lParam;

	message_queue.push_back(message);

	return true;
}

DEVILUTION_END_NAMESPACE
