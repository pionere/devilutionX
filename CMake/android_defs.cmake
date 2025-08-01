# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

set(PIE ON)

# General build options.
# The android build gives Permission denied error if stripping is enabled.
set(DISABLE_STRIP ON)
set(FULLSCREEN_ONLY ON)
set(VIRTUAL_GAMEPAD ON)
#set(ZEROTIER OFF)
# Disable system dependencies.
# All of these will be fetched via FetchContent and linked statically.
set(DEVILUTIONX_SYSTEM_SDL2 OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

# Package the assets with the APK.
#set(BUILD_ASSETS_MPQ OFF)
#set(DEVILUTIONX_ASSETS_OUTPUT_DIRECTORY "${DevilutionX_SOURCE_DIR}/android-project/app/src/main/assets")

if(BINARY_RELEASE OR CMAKE_BUILD_TYPE STREQUAL "Release")
  # Work around a linker bug in clang: https://github.com/android/ndk/issues/721
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=full")
endif()

# Additional compilation definitions
#set(TTF_FONT_DIR \"\")
