# General build options.
#set(VIRTUAL_GAMEPAD ON)
set(ZEROTIER OFF)

# Disable all system dependencies.
# All of these will be fetched via FetchContent and linked statically.
set(DEVILUTIONX_SYSTEM_SDL2 OFF)
set(DEVILUTIONX_STATIC_SDL2 OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)

# Package the assets with the APK.
#set(BUILD_ASSETS_MPQ OFF)
#set(DEVILUTIONX_ASSETS_OUTPUT_DIRECTORY "${DevilutionX_SOURCE_DIR}/android-project/app/src/main/assets")

# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

if(BINARY_RELEASE OR CMAKE_BUILD_TYPE STREQUAL "Release")
  # Work around a linker bug in clang: https://github.com/android/ndk/issues/721
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=full")
endif()

#additional compilation definitions
#set(TTF_FONT_DIR \"\")
