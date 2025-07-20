# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
# The Vita build needs the information
set(DISABLE_STRIP ON)
set(FULLSCREEN_ONLY ON)
set(NONET ON)
#set(ZEROTIER OFF)
# Disable system dependencies.
#set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

set(PREFILL_PLAYER_NAME ON)
