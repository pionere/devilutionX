# Compiling on an Android device itself, e.g. via Termux.
#
# In this scenario, Android is more like a regular Linux platform,
# and the NDK is not available.
set(TERMUX ON)

# General build options.
set(ZEROTIER OFF)
set(FULLSCREEN_ONLY ON)
# Disable system dependencies.
# All of these will be fetched via FetchContent and linked statically.
set(DEVILUTIONX_SYSTEM_SDL2 OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)
