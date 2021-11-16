#General compilation options
set(ASAN OFF)
set(UBSAN OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)
set(ZEROTIER OFF)
set(USE_SDL1 ON)
set(PREFILL_PLAYER_NAME ON)

#3DS libraries
list(APPEND CMAKE_MODULE_PATH "${DevilutionX_SOURCE_DIR}/CMake/ctr/modules")
find_package(CITRO3D REQUIRED)
find_package(FREETYPE REQUIRED)
find_package(BZIP2 REQUIRED)
find_package(Tremor REQUIRED)
find_package(OGG REQUIRED)
find_package(MIKMOD REQUIRED)
find_package(MAD REQUIRED)
find_package(PNG REQUIRED)

#additional compilation definitions
add_definitions(-D__3DS__)
set(TTF_FONT_DIR \"romfs:/\")

#SDL video mode parameters
set(SDL1_VIDEO_MODE_BPP 8)
set(DEFAULT_WIDTH 800)
set(DEFAULT_HEIGHT 480)

#SDL Joystick axis mapping (circle-pad/C-stick)
set(JOY_AXIS_LEFTX 0)
set(JOY_AXIS_LEFTY 1)
set(JOY_AXIS_RIGHTX 3)
set(JOY_AXIS_RIGHTY 4)
#SDL Joystick hat mapping (D-pad)
set(JOY_HAT_DPAD_UP_HAT 0)
set(JOY_HAT_DPAD_RIGHT_HAT 0)
set(JOY_HAT_DPAD_DOWN_HAT 0)
set(JOY_HAT_DPAD_LEFT_HAT 0)
set(JOY_HAT_DPAD_UP 1)
set(JOY_HAT_DPAD_RIGHT 2)
set(JOY_HAT_DPAD_DOWN 4)
set(JOY_HAT_DPAD_LEFT 8)
#SDL Joystick button mapping (A / B and X / Y inverted)
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
