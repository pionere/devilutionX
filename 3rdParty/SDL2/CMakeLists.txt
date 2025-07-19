#set(BUILD_SHARED_LIBS ON)
#
#include(FetchContent_MakeAvailableExcludeFromAll)
#include(FetchContent)
#FetchContent_Declare(SDL2
#    URL https://github.com/libsdl-org/SDL/archive/4cd981609b50ed273d80c635c1ca4c1e5518fb21.tar.gz
#    URL_HASH MD5=b805579e8bf30dcc543eced3686ee72e
#)
#FetchContent_MakeAvailableExcludeFromAll(SDL2)
#
#add_library(SDL2::SDL2 ALIAS SDL2)
if(DEVILUTIONX_STATIC_SDL2)
  set(BUILD_SHARED_LIBS OFF)
  set(SDL_SHARED OFF)
  set(SDL_STATIC ON)
  if(PIE)
    set(SDL_STATIC_PIC ON)
  endif()
else()
  set(BUILD_SHARED_LIBS ON)
  set(SDL_SHARED ON)
  set(SDL_STATIC OFF)
endif()
# Temporary workaround for MSVC builds
if(MSVC)
  set(SDL_LIBC ON)
endif()
set(SDL_DYNAPI OFF)
# Turn off unused subsystems
set(SDL_LOCALE OFF)
set(SDL_MISC OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)
set(SDL_FILE OFF)
set(SDL_DISKAUDIO OFF)
set(SDL_DUMMYAUDIO OFF)
set(SDL_DUMMYVIDEO OFF)
#set(SDL_LEAN_AND_MEAN ON) # requires fork of SDL2
set(SDL_DYN_MEMFUNCS OFF) # requires fork of SDL2
set(SDL_BLIT_0 OFF) # requires fork of SDL2
#set(SDL_BLIT_1 OFF) # requires fork of SDL2
set(SDL_BLIT_A OFF) # requires fork of SDL2
set(SDL_BLIT_N OFF) # requires fork of SDL2
set(SDL_BLIT_N_RGB565 OFF) # requires fork of SDL2
set(SDL_BLIT_AUTO OFF) # requires fork of SDL2
set(SDL_BLIT_SLOW OFF) # requires fork of SDL2
set(SDL_RLE_ACCEL OFF) # requires fork of SDL2
set(SDL_BLIT_TRANSFORM OFF) # requires fork of SDL2
set(SDL_YUV_FORMAT OFF) # requires fork of SDL2
#set(SDL_RENDER_SW OFF) # requires fork of SDL2
if(NOSOUND)
  set(SDL_AUDIO OFF)
endif()
if(NOT HAS_JOYSTICK AND
   NOT HAS_DPAD AND
   NOT HAS_GAMECTRL AND
   NOT HAS_KBCTRL AND
   NOT HAS_TOUCHPAD)
 set(SDL_JOYSTICK OFF)
 set(SDL_HAPTIC OFF)
 set(SDL_HIDAPI OFF)
endif()
if(NOT DEBUG_MODE)
  set(SDL_ASSERTIONS "disabled") # requires fork of SDL2
  set(SDL_LOGGING OFF) # requires fork of SDL2
  set(SDL_VERBOSE_ERROR OFF) # requires fork of SDL2
  set(SDL_SANITIZE_ACCESS OFF) # requires fork of SDL2
endif()

include(FetchContent_MakeAvailableExcludeFromAll)
include(FetchContent)
FetchContent_Declare(SDL2
    #URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.6/SDL2-2.32.6.tar.gz
    #URL_HASH SHA256=6a7a40d6c2e00016791815e1a9f4042809210bdf10cc78d2c75b45c4f52f93ad
    #URL https://github.com/libsdl-org/SDL/archive/5056b29b0f8611b470d8b8bdb313eab628f8bd6e.tar.gz
    #URL_HASH MD5=3fb6d72c33434082c32d2649c35c6502
    GIT_REPOSITORY https://github.com/pionere/SDL.git
    GIT_TAG 4f4a05f37a557481adabea9fea665f5b4b564bae
)
FetchContent_MakeAvailableExcludeFromAll(SDL2)
