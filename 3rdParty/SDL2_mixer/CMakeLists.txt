#set(BUILD_SHARED_LIBS ON)
#
#include(FetchContent_MakeAvailableExcludeFromAll)
#include(FetchContent)
#
#FetchContent_Declare(SDL2_mixer
#    #URL https://github.com/libsdl-org/SDL_mixer/archive/da75a58c19de9fedea62724a5f7770cbbe39adf9.tar.gz
#    #URL_HASH MD5=c07a8fd27e5beaa62612d53693faf48d
#    URL https://github.com/libsdl-org/SDL_mixer/archive/12265697c94260d90e4247ccccfe25a92e1b4978.tar.gz
#    URL_HASH MD5=641dcb9d49473a5aa176189d053f5e7d
#)
#FetchContent_MakeAvailableExcludeFromAll(SDL2_mixer)
#
## SDL2_mixer only provides an INSTALL_INTERFACE directory
## so use the source directory for the BUILD_INTERFACE
#target_include_directories(SDL2_mixer PUBLIC $<BUILD_INTERFACE:${SDL_mixer_SOURCE_DIR}>)
#
#add_library(SDL2::SDL2_mixer ALIAS SDL2_mixer)


if(DEVILUTIONX_STATIC_SDL2)
  set(BUILD_SHARED_LIBS OFF)
#  set(SDL_SHARED OFF)
#  set(SDL_STATIC ON)
#  if(PIE)
#    set(SDL_STATIC_PIC ON)
#  endif()
else()
  set(BUILD_SHARED_LIBS ON)
#  set(SDL_SHARED ON)
#  set(SDL_STATIC OFF)
endif()

include(FetchContent_MakeAvailableExcludeFromAll)
include(FetchContent)

FetchContent_Declare(SDL2_mixer
    #URL https://github.com/libsdl-org/SDL_mixer/archive/da75a58c19de9fedea62724a5f7770cbbe39adf9.tar.gz
    #URL_HASH MD5=c07a8fd27e5beaa62612d53693faf48d
    URL https://github.com/libsdl-org/SDL_mixer/archive/12265697c94260d90e4247ccccfe25a92e1b4978.tar.gz
    URL_HASH MD5=641dcb9d49473a5aa176189d053f5e7d
)
FetchContent_MakeAvailableExcludeFromAll(SDL2_mixer)

# SDL2_mixer only provides an INSTALL_INTERFACE directory
# so use the source directory for the BUILD_INTERFACE
target_include_directories(SDL2_mixer PUBLIC $<BUILD_INTERFACE:${SDL_mixer_SOURCE_DIR}>)

add_library(SDL2::SDL2_mixer ALIAS SDL2_mixer)

