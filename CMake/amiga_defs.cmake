# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
set(NOMEMCCPY ON)
set(USE_SDL1 ON)
set(NONET ON)
#set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

# SDL video mode parameters
set(SDL1_VIDEO_MODE_BPP 8)
