# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
set(NONET ON)
#set(ZEROTIER OFF)
# Disable system dependencies.
#set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

set(PREFILL_PLAYER_NAME ON)

# SDL Joystick button mapping
set(JOY_BUTTON_DPAD_LEFT 16)
set(JOY_BUTTON_DPAD_UP 17)
set(JOY_BUTTON_DPAD_RIGHT 18)
set(JOY_BUTTON_DPAD_DOWN 19)
