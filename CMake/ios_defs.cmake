enable_language(OBJC)

# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
if(__IPHONEOS__)
  set(FULLSCREEN_ONLY ON)
endif()
#set(VIRTUAL_GAMEPAD ON)
#set(ZEROTIER OFF)
#set(NOSOUND ON)

# Disable system dependencies.
# All of these will be fetched via FetchContent and linked statically.
set(DEVILUTIONX_SYSTEM_SDL2 OFF)
#set(DEVILUTIONX_STATIC_SDL2 ON)
#set(DEVILUTIONX_SYSTEM_SDL_IMAGE OFF)
#set(DEVILUTIONX_SYSTEM_SDL_AUDIOLIB OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)
#set(DEVILUTIONX_SYSTEM_LIBPNG OFF)
#set(DEVILUTIONX_SYSTEM_LIBFMT OFF)
