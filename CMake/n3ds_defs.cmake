# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
# The 3ds build handles the stripping in a custom way.
set(USE_SDL1 ON)
set(DISABLE_STRIP ON)
set(NONET ON)
#set(ZEROTIER OFF)
# Disable system dependencies.
#set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

# additional compilation definitions
add_definitions(-D__3DS__)
#set(TTF_FONT_DIR \"romfs:/\")

# SDL video mode parameters
set(SDL1_VIDEO_MODE_FLAGS SDL_DOUBLEBUF|SDL_HWSURFACE)
set(SDL1_FORCE_SVID_VIDEO_MODE ON)
set(SDL1_VIDEO_MODE_BPP 8)

set(DEFAULT_WIDTH 800)
set(DEFAULT_HEIGHT 480)

set(PREFILL_PLAYER_NAME ON)

# SDL Joystick axis mapping (circle-pad/C-stick)
set(JOY_AXIS_LEFTX 0)
set(JOY_AXIS_LEFTY 1)
set(JOY_AXIS_RIGHTX 2)
set(JOY_AXIS_RIGHTY 3)
# SDL Joystick hat mapping (D-pad)
set(JOY_HAT_DPAD_UP_HAT 0)
set(JOY_HAT_DPAD_RIGHT_HAT 0)
set(JOY_HAT_DPAD_DOWN_HAT 0)
set(JOY_HAT_DPAD_LEFT_HAT 0)
set(JOY_HAT_DPAD_UP 1)
set(JOY_HAT_DPAD_RIGHT 2)
set(JOY_HAT_DPAD_DOWN 4)
set(JOY_HAT_DPAD_LEFT 8)
# SDL Joystick button mapping (A / B and X / Y inverted)
set(JOY_BUTTON_A 2)
set(JOY_BUTTON_B 1)
set(JOY_BUTTON_X 4)
set(JOY_BUTTON_Y 3)
set(JOY_BUTTON_LEFTSHOULDER 5)
set(JOY_BUTTON_RIGHTSHOULDER 6)
set(JOY_BUTTON_BACK 7)
set(JOY_BUTTON_START 0)
set(JOY_BUTTON_TRIGGERLEFT 8)
set(JOY_BUTTON_TRIGGERRIGHT 9)
