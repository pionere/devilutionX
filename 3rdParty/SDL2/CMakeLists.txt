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

include(FetchContent_MakeAvailableExcludeFromAll)
include(FetchContent)
FetchContent_Declare(SDL2
    #URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.0.18.tar.gz
    #URL_HASH MD5=647e2890385e5f13b9aeee2e27fa26e7
    URL https://github.com/libsdl-org/SDL/archive/5056b29b0f8611b470d8b8bdb313eab628f8bd6e.tar.gz
    URL_HASH MD5=3fb6d72c33434082c32d2649c35c6502
)
FetchContent_MakeAvailableExcludeFromAll(SDL2)

if(TARGET SDL2::SDL2-static)
  add_library(SDL2::SDL2 ALIAS SDL2-static)
endif()
