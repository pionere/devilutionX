#pragma once

#include "../all.h"

DEVILUTION_BEGIN_NAMESPACE

extern WNDPROC CurrentWndProc;

void SetCursorPos(int X, int Y);

bool PeekMessage(Dvl_Event &event);

#if defined(USE_SDL1) || DEBUG_MODE
int TranslateKey2Char(int vkey);
#endif
void DispatchMessage(const Dvl_Event* event);
void PostMessage(UINT Msg /*, WPARAM wParam*/);
WNDPROC SetWindowProc(WNDPROC NewProc);

//
// Events
//
typedef enum window_messages {
	DVL_WM_NONE,
	DVL_WM_QUIT,
	DVL_WM_MOUSEMOVE,
	DVL_WM_LBUTTONDOWN,
	DVL_WM_LBUTTONUP,
	DVL_WM_RBUTTONDOWN,
	DVL_WM_RBUTTONUP,

	DVL_WM_KEYDOWN,
	DVL_WM_KEYUP,
	DVL_WM_TEXT,

	DVL_WM_CAPTURECHANGED,
	DVL_WM_PAINT,
	// DVL_WM_QUERYENDSESSION,

	DVL_DWM_NEXTLVL,  // dungeon -> next level
	DVL_DWM_PREVLVL,  // dungeon -> previous level
	DVL_DWM_SETLVL,   // dungeon -> setlevel
	DVL_DWM_RTNLVL,   // setlevel -> dungeon
	DVL_DWM_DYNLVL,   // town -> custom dungeon
	DVL_DWM_PORTLVL,  // portal (town <-> dungeon)
	DVL_DWM_TWARPDN,  // town -> dungeon
	DVL_DWM_TWARPUP,  // dungeon -> town
	DVL_DWM_RETOWN,   // restart in town
	DVL_DWM_NEWGAME,  // new game
	DVL_DWM_LOADGAME, // load game

	NUM_WNDMSGS
} window_messages;

//#define DVL_SC_CLOSE 0xF060

// Virtual key codes.
//
// ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define DVL_VK_LBUTTON    0x01 // Left-Mouse button
#define DVL_VK_RBUTTON    0x02 // Right-Mouse button
#define DVL_VK_CANCEL     0x03 // Cancel key
#define DVL_VK_MBUTTON    0x04 // Middle-Mouse button
#define DVL_VK_XBUTTON1   0x05 // X1-Mouse button
#define DVL_VK_BACK       0x08 // BACKSPACE key
#define DVL_VK_TAB        0x09 // TAB key
#define DVL_VK_RETURN     0x0D // ENTER key
#define DVL_VK_SHIFT      0x10 // SHIFT key
#define DVL_VK_CONTROL    0x11 // CONTROL key
#define DVL_VK_MENU       0x12 // ALT key
#define DVL_VK_PAUSE      0x13 // PAUSE key
#define DVL_VK_CAPITAL    0x14 // CAPS LOCK key
#define DVL_VK_HANGUL     0x15 // IME Hangul mode
#define DVL_VK_IME_ON     0x16 // IME On
#define DVL_VK_JUNJA      0x17 // IME Junja mode
#define DVL_VK_FINAL      0x18 // IME final mode
#define DVL_VK_HANJA      0x19 // IME Hanja mode
#define DVL_VK_IME_OFF    0x1A // IME Off
#define DVL_VK_ESCAPE     0x1B // ESC key
#define DVL_VK_MODECHANGE 0x1E // IME mode change request
#define DVL_VK_SPACE      0x20 // SPACEBAR
#define DVL_VK_PRIOR      0x21 // PAGE UP key
#define DVL_VK_NEXT       0x22 // PAGE DOWN key
#define DVL_VK_END        0x23 // END key
#define DVL_VK_HOME       0x24 // HOME key
#define DVL_VK_LEFT       0x25 // LEFT ARROW key
#define DVL_VK_UP         0x26 // UP ARROW key
#define DVL_VK_RIGHT      0x27 // RIGHT ARROW key
#define DVL_VK_DOWN       0x28 // DOWN ARROW key
#define DVL_VK_SELECT     0x29 // SELECT key
#define DVL_VK_PRINT      0x2A // PRINT key
#define DVL_VK_EXECUTE    0x2B // EXECUTE key
#define DVL_VK_SNAPSHOT   0x2C // PRINT SCREEN key
#define DVL_VK_INSERT     0x2D // INS key
#define DVL_VK_DELETE     0x2E // DEL key
#define DVL_VK_HELP       0x2F // Help key
// DVL_VK_0 through DVL_VK_9 correspond to '0' - '9'
// DVL_VK_A through DVL_VK_Z correspond to 'A' - 'Z'
#define DVL_VK_0                   0x30
#define DVL_VK_1                   0x31
#define DVL_VK_2                   0x32
#define DVL_VK_3                   0x33
#define DVL_VK_4                   0x34
#define DVL_VK_5                   0x35
#define DVL_VK_6                   0x36
#define DVL_VK_7                   0x37
#define DVL_VK_8                   0x38
#define DVL_VK_9                   0x39
#define DVL_VK_A                   0x41
#define DVL_VK_B                   0x42
#define DVL_VK_C                   0x43
#define DVL_VK_D                   0x44
#define DVL_VK_E                   0x45
#define DVL_VK_F                   0x46
#define DVL_VK_G                   0x47
#define DVL_VK_I                   0x49
#define DVL_VK_L                   0x4C
#define DVL_VK_M                   0x4D
#define DVL_VK_P                   0x50
#define DVL_VK_Q                   0x51
#define DVL_VK_R                   0x52
#define DVL_VK_S                   0x53
#define DVL_VK_U                   0x55
#define DVL_VK_V                   0x56
#define DVL_VK_W                   0x57
#define DVL_VK_X                   0x58
#define DVL_VK_Z                   0x5A
#define DVL_VK_LWIN                0x5B // Left Windows key (Natural keyboard)
#define DVL_VK_RWIN                0x5C // Right Windows key (Natural keyboard)
#define DVL_VK_APPS                0x5D // Applications key (Natural keyboard)
#define DVL_VK_SLEEP               0x5F // Computer Sleep key
#define DVL_VK_NUMPAD0             0x60 // Numeric keypad 0 key
#define DVL_VK_NUMPAD1             0x61 // Numeric keypad 1 key
#define DVL_VK_NUMPAD2             0x62 // Numeric keypad 2 key
#define DVL_VK_NUMPAD3             0x63 // Numeric keypad 3 key
#define DVL_VK_NUMPAD4             0x64 // Numeric keypad 4 key
#define DVL_VK_NUMPAD5             0x65 // Numeric keypad 5 key
#define DVL_VK_NUMPAD6             0x66 // Numeric keypad 6 key
#define DVL_VK_NUMPAD7             0x67 // Numeric keypad 7 key
#define DVL_VK_NUMPAD8             0x68 // Numeric keypad 8 key
#define DVL_VK_NUMPAD9             0x69 // Numeric keypad 9 key
#define DVL_VK_MULTIPLY            0x6A // Multiply key
#define DVL_VK_ADD                 0x6B // Add key
#define DVL_VK_SEPARATOR           0x6C // Separator key
#define DVL_VK_SUBTRACT            0x6D // Subtract key
#define DVL_VK_DECIMAL             0x6E // Decimal key
#define DVL_VK_DIVIDE              0x6F // Divide key
#define DVL_VK_F1                  0x70 // F1 key
#define DVL_VK_F2                  0x71 // F2 key
#define DVL_VK_F3                  0x72 // F3 key
#define DVL_VK_F4                  0x73 // F4 key
#define DVL_VK_F5                  0x74 // F5 key
#define DVL_VK_F6                  0x75 // F6 key
#define DVL_VK_F7                  0x76 // F7 key
#define DVL_VK_F8                  0x77 // F8 key
#define DVL_VK_F9                  0x78 // F9 key
#define DVL_VK_F10                 0x79 // F10 key
#define DVL_VK_F11                 0x7A // F11 key
#define DVL_VK_F12                 0x7B // F12 key
#define DVL_VK_F13                 0x7C // F13 key
#define DVL_VK_F24                 0x87 // F24 key
#define DVL_VK_NUMLOCK             0x90 // NUM LOCK key
#define DVL_VK_SCROLL              0x91 // SCROLL LOCK key
#define DVL_VK_LSHIFT              0xA0 // Left SHIFT key
#define DVL_VK_RSHIFT              0xA1 // Right SHIFT key
#define DVL_VK_LCONTROL            0xA2 // Left CONTROL key
#define DVL_VK_RCONTROL            0xA3 // Right CONTROL key
#define DVL_VK_LMENU               0xA4 // Left MENU key
#define DVL_VK_RMENU               0xA5 // Right MENU key
#define DVL_VK_BROWSER_BACK        0xA6 // Browser Back key
#define DVL_VK_BROWSER_FORWARD     0xA7 // Browser Forward key
#define DVL_VK_BROWSER_REFRESH     0xA8 // Browser Refresh key
#define DVL_VK_BROWSER_STOP        0xA9 // Browser Stop key
#define DVL_VK_BROWSER_SEARCH      0xAA // Browser Search key
#define DVL_VK_BROWSER_FAVORITES   0xAB // Browser Favorites key
#define DVL_VK_BROWSER_HOME        0xAC // Browser Start and Home key
#define DVL_VK_VOLUME_MUTE         0xAD // Volume mute
#define DVL_VK_VOLUME_DOWN         0xAE // Volume down
#define DVL_VK_VOLUME_UP           0xAF // Volume up
#define DVL_VK_MEDIA_NEXT_TRACK    0xB0 // Next Track key
#define DVL_VK_MEDIA_PREV_TRACK    0xB1 // Previous Track key
#define DVL_VK_MEDIA_STOP          0xB2 // Stop Media key
#define DVL_VK_MEDIA_PLAY_PAUSE    0xB3 // Play/Pause Media key
#define DVL_VK_LAUNCH_MAIL         0xB4 // Start Mail key
#define DVL_VK_LAUNCH_MEDIA_SELECT 0xB5 // Select Media key
#define DVL_VK_LAUNCH_APP1         0xB6 // Start Application 1 key
#define DVL_VK_LAUNCH_APP2         0xB7 // Start Application 2 key
#define DVL_VK_OEM_1               0xBA // For the US standard keyboard, the ':' key
#define DVL_VK_OEM_PLUS            0xBB // For any country/region, the '+' key
#define DVL_VK_OEM_COMMA           0xBC // For any country/region, the ',' key
#define DVL_VK_OEM_MINUS           0xBD // For any country/region, the '-' key
#define DVL_VK_OEM_PERIOD          0xBE // For any country/region, the '.' key
#define DVL_VK_OEM_2               0xBF // For the US standard keyboard, the '/?' key
#define DVL_VK_OEM_3               0xC0 // For the US standard keyboard, the '`~' key
// -- non standard virtual key-codes for the controllers
#define DVL_VK_CONTROLLER_1        0xC1
#define DVL_VK_CONTROLLER_2        0xC2
#define DVL_VK_CONTROLLER_3        0xC3
#define DVL_VK_CONTROLLER_4        0xC4
// --
#define DVL_VK_OEM_4               0xDB // For the US standard keyboard, the '[{' key
#define DVL_VK_OEM_5               0xDC // For the US standard keyboard, the '\|' key
#define DVL_VK_OEM_6               0xDD // For the US standard keyboard, the ']}' key
#define DVL_VK_OEM_7               0xDE // For the US standard keyboard, the 'single-quote/double-quote' key
#define DVL_VK_OEM_8               0xDF // -
#define DVL_VK_OEM_9               0xE1 // OEM specific
#define DVL_VK_OEM_102             0xE2 // angle bracket/backslash on the RT 102-key keyboard
#define DVL_VK_OEM_10              0xE3 // OEM specific
#define DVL_VK_OEM_11              0xE4 // OEM specific

#define DVL_VK_OEM_12 0xE9 // OEM specific
#define DVL_VK_OEM_13 0xEA // OEM specific
#define DVL_VK_OEM_14 0xEB // OEM specific
#define DVL_VK_OEM_15 0xEC // OEM specific
#define DVL_VK_OEM_16 0xED // OEM specific
#define DVL_VK_OEM_17 0xEE // OEM specific
#define DVL_VK_OEM_18 0xEF // OEM specific
#define DVL_VK_OEM_19 0xF0 // OEM specific
#define DVL_VK_OEM_20 0xF1 // OEM specific
#define DVL_VK_OEM_21 0xF2 // OEM specific
#define DVL_VK_OEM_22 0xF3 // OEM specific
#define DVL_VK_OEM_23 0xF4 // OEM specific
#define DVL_VK_OEM_24 0xF5 // OEM specific
#define DVL_VK_ATTN   0xF6 // Attn key
#define DVL_VK_CRSEL  0xF7 // CrSel key
#define DVL_VK_EXSEL  0xF8 // ExSel key
#define DVL_VK_EREOF  0xF9 // Erase EOF key
#define DVL_VK_PLAY   0xFA // Play key
#define DVL_VK_ZOOM   0xFB // Zoom key
#define DVL_VK_NONAME 0xFC // Reserved
#define DVL_VK_PA1    0xFD // PA1 key

#define DVL_VK_OEM_CLEAR 0xFE // Clear button

#define DVL_MK_SHIFT   0x0004
#define DVL_MK_LBUTTON 0x0001
#define DVL_MK_RBUTTON 0x0002

DEVILUTION_END_NAMESPACE
