cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()
if(POLICY CMP0111)
  cmake_policy(SET CMP0111 NEW)
endif()

# Projects added via `add_subdirectory` or `FetchContent` may have a lower
# `cmake_minimum_required` than we set here. Set policies that we require
# to their new value so that they still apply.
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
if(NINTENDO_3DS)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ctr")
endif()

if(NOT VERSION_NUM)
  include(git)
  get_git_tag(VERSION_NUM)
  string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" VERSION_NUM ${VERSION_NUM} )
  if("${VERSION_NUM}" STREQUAL "")
    set(VERSION_NUM untagged)
  endif()
  if(NOT VERSION_SUFFIX)
    get_git_commit_hash(GIT_COMMIT_HASH)
    set(VERSION_SUFFIX "$<$<NOT:$<CONFIG:Release>>:-${GIT_COMMIT_HASH}>")
  endif()
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(VERSION_NUM MATCHES untagged)
  project(DevilutionX
    LANGUAGES C CXX)
else()
  project(DevilutionX
    VERSION ${VERSION_NUM}
    LANGUAGES C CXX)
endif()

if(NINTENDO_SWITCH)
  include(switch_defs)
endif()

if(VITA)
  include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
  include(vita_defs)
endif()

set(TARGET_PLATFORM host CACHE STRING "Target platform")
set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS host lepus retrofw rg350 gkd350h cpigamesh)
if(TARGET_PLATFORM STREQUAL "lepus")
  include(lepus_defs)
elseif(TARGET_PLATFORM STREQUAL "retrofw")
  include(retrofw_defs)
elseif(TARGET_PLATFORM STREQUAL "rg350")
  include(rg350_defs)
elseif(TARGET_PLATFORM STREQUAL "gkd350h")
  include(gkd350h_defs)
elseif(TARGET_PLATFORM STREQUAL "cpigamesh")
  include(cpigamesh_defs)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD|OpenBSD|DragonFly|NetBSD")
  if(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    add_definitions(-D_NETBSD_SOURCE)
  else()
    add_definitions(-D_BSD_SOURCE)
    set(UBSAN OFF)
  endif()
  set(ASAN OFF)
endif()

if(WIN32)
  set(ASAN OFF)
  set(UBSAN OFF)
  if(_WIN32_WINNT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=${_WIN32_WINNT}")
  endif()
endif()

if(HAIKU)
  set(ASAN OFF)
  set(UBSAN OFF)
endif()

if(AMIGA)
  include(amiga_defs)
endif()

if(NINTENDO_3DS)
  include(n3ds_defs)
endif()

if(PS4)
  include(ps4_defs)
endif()

if(ANDROID)
  include(android_defs)
endif()

if(IOS)
  include(ios_defs)
endif()

include(CMakeDependentOption)
include(out_of_tree)

macro(cmake_conditional_option NAME DESCRIPTION)
  if(${ARGN})
    option(${NAME} ${DESCRIPTION} ON)
  else()
    option(${NAME} ${DESCRIPTION} OFF)
  endif()
endmacro()
macro(cmake_debug_option NAME DESCRIPTION)
  cmake_conditional_option(${NAME} ${DESCRIPTION} CMAKE_BUILD_TYPE STREQUAL "Debug")
endmacro()
macro(cmake_release_option NAME DESCRIPTION)
  cmake_conditional_option(${NAME} ${DESCRIPTION} NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
endmacro()

cmake_debug_option(ASAN "Enable address sanitizer")
cmake_debug_option(UBSAN "Enable undefined behaviour sanitizer")
cmake_dependent_option(TSAN "Enable thread sanitizer" OFF "NOT ASAN" OFF)
cmake_debug_option(DEBUG_MODE "Enable debug mode in engine")
option(DEV_MODE "Enable development mode in engine" OFF)
option(GPERF "Build with GPerfTools profiler" OFF)
cmake_dependent_option(GPERF_HEAP_FIRST_GAME_ITERATION "Save heap profile of the first game iteration" OFF "GPERF" OFF)
option(DISABLE_LTO "Disable link-time optimization (by default enabled in release mode)" OFF)
option(PIE "Generate position-independent code" OFF)
option(CPACK "Configure CPack" ON)
option(USE_LD_MOLD "Use mold linker(GNU|Clang)" OFF)
option(MACOSX_STANDALONE_APP_BUNDLE "Generate a portable app bundle to use on other devices (requires sudo)" OFF)
option(USE_SDL1 "Use SDL1.2 instead of SDL2" OFF)
option(NOSOUND "Disable sound support" OFF)
option(STREAM_ALL_AUDIO "Stream all the audio. For extremely RAM-constrained platforms." OFF)
option(NOWIDESCREEN "Disable widescreen support" OFF)
option(NONET "Disable network support" OFF)
cmake_dependent_option(INET_MODE "Enable validation of network messages" OFF "NOT NONET" OFF)
cmake_dependent_option(ADAPTIVE_NETUPDATE "Build adaptive network support" ON "NOT NONET" OFF)
cmake_dependent_option(NETENCRYPT "Encrypt network messages" ON "NOT NONET" OFF)
cmake_dependent_option(TCPIP "Build with tcp/ip support" ON "NOT NONET" OFF)
cmake_dependent_option(ZEROTIER "Build with zerotier support" OFF "NOT NONET" OFF)
cmake_dependent_option(NOHOSTING "Disable support for host-only games" ON "TCPIP" ON)
cmake_dependent_option(HOSTONLY "Disable support for non host-only games" OFF "TCPIP AND NOT NOHOSTING" OFF)
cmake_dependent_option(DISABLE_EXCEPTIONS "Disable exceptions" ON "NONET" OFF)
option(RUN_TESTS "Build and run tests" OFF)
option(HELLFIRE "Build hellfire version" OFF)
option(HAS_JOYSTICK "Enable joystick support" ON)
option(HAS_DPAD "Enable D-Pad support" ON)
option(HAS_KBCTRL "Enable keyboard controller support" ON)
cmake_dependent_option(HAS_GAMECTRL "Enable game controller support" ON "NOT USE_SDL1" OFF)
cmake_dependent_option(HAS_TOUCHPAD "Enable touchpad support" ON "NOT USE_SDL1" OFF)

if(NOT NONET)
  option(DEVILUTIONX_SYSTEM_LIBSODIUM "Use system-provided libsodium" ON)
  cmake_conditional_option(DEVILUTIONX_STATIC_LIBSODIUM "Link static libsodium" NOT ${DEVILUTIONX_SYSTEM_LIBSODIUM})
endif()

if(NOT USE_SDL1)
  option(DEVILUTIONX_SYSTEM_SDL2 "Use system-provided SDL2" ON)
  cmake_conditional_option(DEVILUTIONX_STATIC_SDL2 "Link static SDL2" NOT ${DEVILUTIONX_SYSTEM_SDL2})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
if(PIE)
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

# Remove unused symbols in non-debug mode.
# This is useful even with LTO (-84 KiB with MinSizeRel).
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # For some reason, adding to CMAKE_CXX_FLAGS results in a slightly smaller
  # binary than using `add_compile/link_options`
  set(_extra_flags "-ffunction-sections -fdata-sections")
  if(APPLE)
    set(_extra_flags "${_extra_flags} -Wl,-dead_strip")
  else()
    set(_extra_flags "${_extra_flags} -Wl,--gc-sections,--as-needed")
  endif()

  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${_extra_flags}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${_extra_flags}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${_extra_flags}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${_extra_flags}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${_extra_flags}")
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${_extra_flags}")

  if(USE_LD_MOLD)
    if(NOT CMAKE_CROSSCOMPILING)
      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
        find_program(
          LD_MOLD_PATH
          ld
          PATHS
          ${CMAKE_INSTALL_PREFIX}/libexec/mold
          ENV LD_MOLD_PATH
          NO_DEFAULT_PATH
        )
        if(NOT LD_MOLD_PATH STREQUAL "LD_MOLD_PATH-NOTFOUND")
          get_filename_component(_mold_dir ${LD_MOLD_PATH} DIRECTORY)
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -B${_mold_dir}")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -B${_mold_dir}")
        else()
          message(WARNING "-- Cannot use mold linker: mold ld directory not found")
        endif()
      else()
      #elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=mold")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=mold")
      endif()
    else()
      message(WARNING "-- Mold linker is disabled due to CMAKE_CROSSCOMPILING")
    endif()
  endif()
endif()

# Not a genexp because CMake doesn't support it
# https://gitlab.kitware.com/cmake/cmake/-/issues/20546
if(NOT DISABLE_LTO)
  # LTO if supported:
  include(CheckIPOSupported)
  check_ipo_supported(RESULT is_ipo_supported OUTPUT lto_error)
  if(is_ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
  endif()
endif()

find_package(Threads REQUIRED)
if(NINTENDO_3DS)
  find_package(CITRO3D REQUIRED)
  find_package(FREETYPE REQUIRED)
  find_package(BZIP2 REQUIRED)
  find_package(Tremor REQUIRED)
  find_package(OGG REQUIRED)
  find_package(MIKMOD REQUIRED)
  find_package(MAD REQUIRED)
  find_package(PNG REQUIRED)
endif()
if(AMIGA)
  find_package(Freetype REQUIRED)
  find_package(ZLIB REQUIRED)
endif()

if(NOT NONET)
  if(DEVILUTIONX_SYSTEM_LIBSODIUM)
    set(sodium_USE_STATIC_LIBS ${DEVILUTIONX_STATIC_LIBSODIUM})
    find_package(sodium REQUIRED)
  else()
    add_subdirectory(3rdParty/libsodium)
  endif()
endif()

if(USE_SDL1)
  find_package(SDL REQUIRED)
  include_directories(${SDL_INCLUDE_DIR})
elseif(DEVILUTIONX_SYSTEM_SDL2)
  find_package(SDL2 REQUIRED)
  if(TARGET SDL2::SDL2)
    set(SDL2_MAIN SDL2::SDL2main)
  elseif(TARGET SDL2::SDL2-static)
    # On some distros, such as vitasdk, only the SDL2::SDL2-static target is available.
    # Alias to SDL2::SDL2 because some finder scripts may refer to SDL2::SDL2.
    if(CMAKE_VERSION VERSION_LESS "3.18")
      # Aliasing local targets is not supported on CMake < 3.18, so make it global.
      set_target_properties(SDL2::SDL2-static PROPERTIES IMPORTED_GLOBAL TRUE)
    endif()
    add_library(SDL2::SDL2 ALIAS SDL2::SDL2-static)
    set(SDL2_MAIN SDL2::SDL2main)
  else()
    # Assume an older Debian derivate that comes with an sdl2-config.cmake
    # that only defines `SDL2_LIBRARIES` (as -lSDL2) and `SDL2_INCLUDE_DIRS`.
    add_library(SDL2_lib INTERFACE)
    target_link_libraries(SDL2_lib INTERFACE ${SDL2_LIBRARIES})
    target_include_directories(SDL2_lib INTERFACE ${SDL2_INCLUDE_DIRS})
    # Can't define an INTERFACE target with ::, so alias instead
    add_library(SDL2::SDL2 ALIAS SDL2_lib)
  endif()
else()
  add_subdirectory(3rdParty/SDL2)
  set(SDL2_MAIN SDL2::SDL2main)
endif()

if(NOT NOSOUND)
  add_library(SDL2_mixer STATIC
    3rdParty/SDL2_mixer/src/effect_position.c
    3rdParty/SDL2_mixer/src/effects_internal.c
    3rdParty/SDL2_mixer/src/mixer.c
    3rdParty/SDL2_mixer/src/music.c
    3rdParty/SDL2_mixer/src/utils.c
    3rdParty/SDL2_mixer/src/codecs/music_wav.c)
  target_compile_definitions(SDL2_mixer PRIVATE MUSIC_WAV)
  if(USE_SDL1)
    target_link_libraries(SDL2_mixer PRIVATE ${SDL_LIBRARY})
  else()
    target_link_libraries(SDL2_mixer PRIVATE SDL2::SDL2)
  endif()
  # include with 'BEFORE' to handle 'smart' environments where SDL(2)_INCLUDE_DIR
  # contains SDL(2)_mixer...
  #target_include_directories(SDL2_mixer PUBLIC 3rdParty/SDL2_mixer/include)
  include_directories(BEFORE PUBLIC 3rdParty/SDL2_mixer/include)
  set(SDL_MIXER_LIBRARY SDL2_mixer)
endif()

if(TCPIP)
  add_subdirectory(3rdParty/asio)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_TYPEID ASIO_DISABLE_FILE ASIO_CONCURRENCY_HINT_DEFAULT=1)
  if(NINTENDO_3DS)
    target_compile_definitions(asio INTERFACE ASIO_DISABLE_THREADS=ON ASIO_HAS_UNISTD_H=ON)
    # Missing headers and declarations provided by DevilutionX
    target_include_directories(asio BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Source/platform/ctr/asio/include)
  elseif(NINTENDO_SWITCH)
    # Enables a number of header file definitions required by ASIO
    target_compile_definitions(asio INTERFACE _DEFAULT_SOURCE=ON)
    # Missing headers and declarations provided by DevilutionX
    target_include_directories(asio BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Source/platform/switch/asio/include)
  endif()
endif()

if(ZEROTIER)
  add_subdirectory(3rdParty/libzt)
endif()

add_library(smacker STATIC
  3rdParty/libsmacker/smacker.c)
target_include_directories(smacker PUBLIC 3rdParty/libsmacker)

#if(WIN32)
#  add_subdirectory(3rdParty/find_steam_game)
#endif()

add_library(Radon STATIC
  3rdParty/Radon/Radon/source/File.cpp
  3rdParty/Radon/Radon/source/Key.cpp
  3rdParty/Radon/Radon/source/Named.cpp
  3rdParty/Radon/Radon/source/Section.cpp)
target_include_directories(Radon PUBLIC 3rdParty/Radon/Radon/include)

add_library(StormLib STATIC
  3rdParty/StormLib/src/FileStream.cpp
  3rdParty/StormLib/src/SBaseCommon.cpp
  3rdParty/StormLib/src/SBaseFileTable.cpp
  3rdParty/StormLib/src/SBaseSubTypes.cpp
  3rdParty/StormLib/src/SCompression.cpp
  3rdParty/StormLib/src/SFileExtractFile.cpp
  3rdParty/StormLib/src/SFileFindFile.cpp
  3rdParty/StormLib/src/SFileGetFileInfo.cpp
  3rdParty/StormLib/src/SFileOpenArchive.cpp
  3rdParty/StormLib/src/SFileOpenFileEx.cpp
  3rdParty/StormLib/src/SFileReadFile.cpp)

#if(WIN32)
## Enable Unicode for StormLib wchar_t* file APIs
#target_compile_definitions(StormLib PRIVATE -DUNICODE -D_UNICODE)
#endif()

add_library(PKWare STATIC
  3rdParty/PKWare/explode.cpp
  3rdParty/PKWare/implode.cpp)
target_include_directories(PKWare PUBLIC 3rdParty/PKWare)

set(devilutionx_SRCS
  Source/appfat.cpp
  Source/automap.cpp
  Source/capture.cpp
  Source/codec.cpp
  Source/control.cpp
  Source/cursor.cpp
  Source/debug.cpp
  Source/diablo.cpp
  #Source/doom.cpp
  Source/drlg_l1.cpp
  Source/drlg_l2.cpp
  Source/drlg_l3.cpp
  Source/drlg_l4.cpp
  Source/dthread.cpp
  Source/dx.cpp
  Source/effects.cpp
  Source/encrypt.cpp
  Source/engine.cpp
  Source/error.cpp
  Source/gamemenu.cpp
  Source/gameui.cpp
  Source/gendung.cpp
  Source/gmenu.cpp
  Source/help.cpp
  Source/init.cpp
  Source/interfac.cpp
  Source/inv.cpp
  Source/itemdat.cpp
  Source/items.cpp
  Source/lighting.cpp
  Source/loadsave.cpp
  Source/main.cpp
  Source/mainmenu.cpp
  Source/minitext.cpp
  Source/misdat.cpp
  Source/missiles.cpp
  Source/monstdat.cpp
  Source/monster.cpp
  Source/movie.cpp
  Source/mpqapi.cpp
  Source/msg.cpp
  Source/multi.cpp
  Source/nthread.cpp
  Source/objdat.cpp
  Source/objects.cpp
  Source/pack.cpp
  Source/palette.cpp
  Source/path.cpp
  Source/pfile.cpp
  Source/player.cpp
  Source/plrmsg.cpp
  Source/portal.cpp
  Source/questdat.cpp
  Source/quests.cpp
  Source/scrollrt.cpp
  Source/setmaps.cpp
  Source/sha.cpp
  Source/sound.cpp
  Source/spelldat.cpp
  Source/spells.cpp
  Source/stores.cpp
  Source/sync.cpp
  Source/textdat.cpp
  Source/themes.cpp
  Source/tmsg.cpp
  Source/town.cpp
  Source/towners.cpp
  Source/trigs.cpp
  Source/controls/axis_direction.cpp
  Source/controls/controller.cpp
  Source/controls/controller_motion.cpp
  Source/controls/devices/game_controller.cpp
  Source/controls/devices/joystick.cpp
  Source/controls/devices/kbcontroller.cpp
  Source/controls/game_controls.cpp
  Source/controls/menu_controls.cpp
  Source/controls/modifier_hints.cpp
  Source/controls/plrctrls.cpp
  Source/controls/touch.cpp
  Source/engine/render/automap_render.cpp
  Source/engine/render/cel_render.cpp
  Source/engine/render/cl2_render.cpp
  Source/engine/render/dun_render.cpp
  Source/engine/render/text_render.cpp
  Source/utils/display.cpp
  Source/utils/paths.cpp
  Source/utils/soundsample.cpp
  Source/utils/thread.cpp
  Source/DiabloUI/credits.cpp
  Source/DiabloUI/credits_lines.cpp
  Source/DiabloUI/diabloui.cpp
  Source/DiabloUI/dialogs.cpp
  Source/DiabloUI/errorart.cpp
  Source/DiabloUI/mainmenu.cpp
  Source/DiabloUI/progress.cpp
  Source/DiabloUI/scrollbar.cpp
  Source/DiabloUI/selconn.cpp
  Source/DiabloUI/selgame.cpp
  Source/DiabloUI/selhero.cpp
  Source/DiabloUI/selok.cpp
  Source/DiabloUI/selyesno.cpp
  Source/DiabloUI/settingsmenu.cpp
  Source/DiabloUI/text.cpp
  Source/DiabloUI/text_draw.cpp
  Source/DiabloUI/title.cpp
  Source/dvlnet/abstract_net.cpp
  Source/dvlnet/base.cpp
  Source/dvlnet/cdwrap.cpp
  Source/dvlnet/frame_queue.cpp
  Source/dvlnet/loopback.cpp
  Source/dvlnet/packet.cpp
  Source/storm/storm.cpp
  Source/storm/storm_net.cpp
  Source/storm/storm_svid.cpp
  Source/miniwin/misc_msg.cpp)

if(IOS)
  list(APPEND devilutionx_SRCS Source/platform/ios/ios_paths.m)
endif()

if(USE_SDL1)
  list(APPEND devilutionx_SRCS Source/utils/sdl2_to_1_2_backports.cpp)
endif()

if(NOT NONET)
  list(APPEND devilutionx_SRCS
    Source/dvlnet/tcp_client.cpp
    Source/dvlnet/tcp_server.cpp
    Source/dvlnet/tcpd_client.cpp
    Source/dvlnet/tcp_host.cpp
    Source/dvlnet/protocol_zt.cpp
    Source/dvlnet/zerotier_native.cpp
    Source/dvlnet/zerotier_lwip.cpp)
endif()

set(BIN_TARGET devilutionx)

if(NINTENDO_SWITCH)
  list(APPEND devilutionx_SRCS
    Source/platform/switch/docking.cpp
    Source/platform/switch/keyboard.cpp
    Source/platform/switch/network.cpp
    Source/platform/switch/random.cpp
    Source/platform/switch/romfs.cpp
    Source/platform/switch/asio/pause.c
    Source/platform/switch/asio/net/if.c
    Source/platform/switch/asio/sys/signal.c)
endif()

if(VITA)
  list(APPEND devilutionx_SRCS
    Source/platform/vita/random.cpp
    Source/platform/vita/network.cpp
    Source/platform/vita/keyboard.cpp)
endif()

if(NINTENDO_3DS)
  list(APPEND devilutionx_SRCS
    Source/platform/ctr/system.cpp
    Source/platform/ctr/keyboard.cpp
    Source/platform/ctr/display.cpp
    Source/platform/ctr/messagebox.cpp
    Source/platform/ctr/random.cpp
    Source/platform/ctr/sockets.cpp
    Source/platform/ctr/asio/net/if.c
    Source/platform/ctr/asio/sys/socket.c
    Source/platform/ctr/asio/sys/uio.c)
  if(HELLFIRE)
    set(BIN_TARGET hellfire)
  endif()
  set(BIN_TARGET ${BIN_TARGET}.elf)
endif()

if(RUN_TESTS)
  list(APPEND devilutionx_SRCS
    test/appfat_test.cpp
    test/automap_test.cpp
    test/control_test.cpp
    test/cursor_test.cpp
    test/codec_test.cpp
    test/diablo_test.cpp
    test/doom_test.cpp
    test/drlg_l1_test.cpp
    test/drlg_l2_test.cpp
    test/drlg_l3_test.cpp
    test/drlg_l4_test.cpp
    test/effects_test.cpp
    test/file_util_test.cpp
    test/lighting_test.cpp
    test/missiles_test.cpp
    test/player_test.cpp
    test/scrollrt_test.cpp
    test/stores_test.cpp)
endif()

if(ANDROID)
  add_library(${BIN_TARGET} SHARED ${devilutionx_SRCS})
elseif(APPLE)
  add_executable(${BIN_TARGET}
    MACOSX_BUNDLE
    ${devilutionx_SRCS}
    Packaging/apple/AppIcon.icns
    Packaging/apple/LaunchScreen.storyboard)
elseif(WIN32)
  add_executable(${BIN_TARGET}
    WIN32
    ${devilutionx_SRCS}
    Packaging/windows/devilutionx.exe.manifest
    Packaging/windows/devilutionx.rc)
else()
  add_executable(${BIN_TARGET}
    WIN32
    MACOSX_BUNDLE
    ${devilutionx_SRCS}
    Packaging/windows/devilutionx.exe.manifest
    Packaging/windows/devilutionx.rc)
endif()

# Copy devilx.mpq to the build directory
file(COPY "Packaging/resources/devilx.mpq" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
if(AMIGA)
  file(COPY "Packaging/amiga/devilutionx.info" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# Use file GENERATE instead of configure_file because configure_file
# does not support generator expressions.
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
  set(CONFIG_PATH $<CONFIG>/config.h)
  target_include_directories(${BIN_TARGET} PRIVATE ${CMAKE_BINARY_DIR}/$<CONFIG>)
else()
  set(CONFIG_PATH config.h)
endif()
file(GENERATE OUTPUT ${CONFIG_PATH} CONTENT
"#pragma once
#define PROJECT_NAME \"${PROJECT_NAME}\"
#define PROJECT_VERSION \"$<IF:$<OR:$<BOOL:${PROJECT_VERSION}>,$<BOOL:${VERSION_SUFFIX}>>,${PROJECT_VERSION}${VERSION_SUFFIX},?>\"
")

if(RUN_TESTS)
  include(CTest)
  include(GoogleTest)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_definitions(-DRUN_TESTS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(-fprofile-arcs -ftest-coverage)
  endif()
  target_link_libraries(${BIN_TARGET} PRIVATE -fprofile-arcs ${GTEST_LIBRARIES})
  gtest_add_tests(${BIN_TARGET} "" AUTO)
endif()

if(GPERF)
  find_package(Gperftools REQUIRED)
  if(GPERF_HEAP_FIRST_GAME_ITERATION)
    set(GPERF_HEAP_MAIN ON)
  endif()

  # Compile with information about file and line numbers for everything
  # even in non-Debug build types.
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options("$<$<NOT:$<CONFIG:Debug>>:-g2>")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Use the more size-efficient `-gmlt` option on clang.
    add_compile_options("$<$<NOT:$<CONFIG:Debug>>:-gmlt>")
  endif()
endif()

target_include_directories(${BIN_TARGET} PRIVATE
  Source
  ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${BIN_TARGET} PUBLIC Threads::Threads)

target_link_libraries(${BIN_TARGET} PRIVATE
  PKWare
  StormLib
  smacker
  Radon)

#if(WIN32)
#  target_link_libraries(libdevilutionx PUBLIC find_steam_game)
#endif()

if(NOT NONET)
  target_link_libraries(${BIN_TARGET} PRIVATE sodium)
endif()
if(TCPIP)
  target_link_libraries(${BIN_TARGET} PRIVATE asio)
endif()
if(ZEROTIER)
  target_link_libraries(${BIN_TARGET} PRIVATE zt-static)
endif()

# Defines without value
foreach(
  def_name
  USE_SDL1
  HELLFIRE
  NOSOUND
  STREAM_ALL_AUDIO
  NOWIDESCREEN
  NONET
  INET_MODE
  NETENCRYPT
  ADAPTIVE_NETUPDATE
  NOHOSTING
  HOSTONLY
  TCPIP
  ZEROTIER
  HAS_GAMECTRL
  HAS_DPAD
  HAS_TOUCHPAD
  HAS_KBCTRL
  HAS_JOYSTICK
  PREFILL_PLAYER_NAME
  DEBUG_MODE
  DEV_MODE
  GPERF
  GPERF_HEAP_MAIN
  GPERF_HEAP_FIRST_GAME_ITERATION
)
if(${def_name})
  list(APPEND def_list ${def_name})
endif()
endforeach(def_name)

# Defines with value
foreach(
  def_name
  ASSET_MPL
  DEFAULT_WIDTH
  DEFAULT_HEIGHT
  SCREEN_WIDTH
  SCREEN_HEIGHT
  #TTF_FONT_DIR
  #TTF_FONT_NAME
  SDL1_VIDEO_MODE_BPP
  SDL1_VIDEO_MODE_FLAGS
  SDL1_VIDEO_MODE_SVID_FLAGS
  SDL1_FORCE_SVID_VIDEO_MODE
  KBCTRL_BUTTON_DPAD_LEFT
  KBCTRL_BUTTON_DPAD_RIGHT
  KBCTRL_BUTTON_DPAD_UP
  KBCTRL_BUTTON_DPAD_DOWN
  KBCTRL_BUTTON_B
  KBCTRL_BUTTON_A
  KBCTRL_BUTTON_Y
  KBCTRL_BUTTON_X
  KBCTRL_BUTTON_LEFTSTICK
  KBCTRL_BUTTON_RIGHTSTICK
  KBCTRL_BUTTON_RIGHTSHOULDER
  KBCTRL_BUTTON_LEFTSHOULDER
  KBCTRL_BUTTON_TRIGGERLEFT
  KBCTRL_BUTTON_TRIGGERRIGHT
  KBCTRL_BUTTON_START
  KBCTRL_BUTTON_BACK
  KBCTRL_IGNORE_1
  JOY_AXIS_LEFTX
  JOY_AXIS_LEFTY
  JOY_AXIS_RIGHTX
  JOY_AXIS_RIGHTY
  JOY_HAT_DPAD_UP_HAT
  JOY_HAT_DPAD_UP
  JOY_HAT_DPAD_DOWN_HAT
  JOY_HAT_DPAD_DOWN
  JOY_HAT_DPAD_LEFT_HAT
  JOY_HAT_DPAD_LEFT
  JOY_HAT_DPAD_RIGHT_HAT
  JOY_HAT_DPAD_RIGHT
  JOY_BUTTON_DPAD_LEFT
  JOY_BUTTON_DPAD_RIGHT
  JOY_BUTTON_DPAD_UP
  JOY_BUTTON_DPAD_DOWN
  JOY_BUTTON_B
  JOY_BUTTON_A
  JOY_BUTTON_Y
  JOY_BUTTON_X
  JOY_BUTTON_LEFTSTICK
  JOY_BUTTON_RIGHTSTICK
  JOY_BUTTON_RIGHTSHOULDER
  JOY_BUTTON_LEFTSHOULDER
  JOY_BUTTON_TRIGGERLEFT
  JOY_BUTTON_TRIGGERRIGHT
  JOY_BUTTON_START
  JOY_BUTTON_BACK
  REMAP_KEYBOARD_KEYS
)
  if(DEFINED ${def_name})
    list(APPEND def_list ${def_name}=${${def_name}})
  endif()
endforeach(def_name)

target_compile_definitions(${BIN_TARGET} PRIVATE ${def_list})
target_compile_definitions(smacker PRIVATE ${def_list})

if(UBSAN)
  target_compile_options(${BIN_TARGET} PUBLIC -fsanitize=undefined)
  target_link_libraries(${BIN_TARGET} PUBLIC -fsanitize=undefined)
endif()

if(TSAN)
  target_compile_options(${BIN_TARGET} PUBLIC -fsanitize=thread)
  target_link_libraries(${BIN_TARGET} PUBLIC -fsanitize=thread)
elseif(ASAN)
  target_compile_options(${BIN_TARGET} PUBLIC "-fsanitize=address;-fsanitize-recover=address")
  target_link_libraries(${BIN_TARGET} PUBLIC "-fsanitize=address;-fsanitize-recover=address")
endif()

if(USE_SDL1)
  target_link_libraries(${BIN_TARGET} PRIVATE
    ${SDL_LIBRARY})
else()
  target_link_libraries(${BIN_TARGET} PRIVATE
    ${SDL2_MAIN}
    SDL2::SDL2)
endif()

if(NOT NOSOUND)
  target_link_libraries(${BIN_TARGET} PRIVATE ${SDL_MIXER_LIBRARY})
endif()

if(AMIGA)
  target_link_libraries(${BIN_TARGET} PRIVATE
    ${FREETYPE_LIBRARY}
    ${ZLIB_LIBRARY})
  if(NOT WARPOS)
    target_link_libraries(${BIN_TARGET} PRIVATE -ldebug)
  endif()
endif()

if(NINTENDO_SWITCH)
  target_link_libraries(${BIN_TARGET} PRIVATE
        freetype
        vorbisfile
        vorbis
        ogg
        modplug
        mpg123
        SDL2
        opusfile
        opus
        png
        bz2
        z
        #EGL
        #glapi
        #drm_nouveau
        #nx
  )
endif()

if(VITA)
  target_link_libraries(${BIN_TARGET} PRIVATE
        ScePower_stub
        SceNet_stub
        SceNetCtl_stub
        freetype
        png
        m
        z
        FLAC
        vorbisfile
        vorbis
        ogg
        mikmod
        mpg123
  )
  #target_compile_definitions(${BIN_TARGET} PRIVATE VITA)
endif()

if(NINTENDO_3DS)
  target_link_libraries(${BIN_TARGET} PRIVATE ${TREMOR_LIBRARIES})
  target_include_directories(${BIN_TARGET} PRIVATE ${TREMOR_INCLUDE_DIRS})
  target_link_libraries(${BIN_TARGET} PRIVATE 3ds::ogg 3ds::mikmod 3ds::mad 3ds::freetype 3ds::bzip2 3ds::png)
  target_link_libraries(${BIN_TARGET} PRIVATE 3ds::citro3d 3ds::ctrulib)
endif()

if(GPERF)
  target_link_libraries(${BIN_TARGET} PRIVATE ${GPERFTOOLS_LIBRARIES})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT GPERF AND NOT HAIKU AND NOT VITA)
  message(STATUS "gnnnuuu set")
  target_link_libraries(${BIN_TARGET} PUBLIC "$<$<NOT:$<CONFIG:Debug>>:-static-libgcc;-static-libstdc++>")
endif()

if(WIN32)
  if(NOT NONET)
    #target_link_libraries(${BIN_TARGET} PRIVATE shlwapi wsock32 ws2_32 wininet)
    target_link_libraries(${BIN_TARGET} PRIVATE wsock32 ws2_32 wininet)
  endif()
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "winnn set")
    target_link_libraries(${BIN_TARGET} PUBLIC "$<$<NOT:$<CONFIG:Debug>>:-static-libgcc;-static-libstdc++>")
    target_link_options(${BIN_TARGET} PRIVATE "-lstdc++_s;-s")
    target_compile_options(${BIN_TARGET} PUBLIC -s)
    target_compile_options(${BIN_TARGET} PUBLIC $<$<CONFIG:Debug>:-gstabs>)
  endif()
endif()

if(NOT WIN32 AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
  # Enable POSIX extensions such as `readlink` and `ftruncate`.
  add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

if(HAIKU)
  target_link_libraries(${BIN_TARGET} PRIVATE network)
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # Change __FILE__ to only show the path relative to the project folder
  if((CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
      OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8))
    target_compile_options(${BIN_TARGET} PUBLIC "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
  else()
    get_target_property(devilutionx_SRCS ${BIN_TARGET} SOURCES)
    foreach(SOURCE_FILE ${devilutionx_SRCS})
      set_source_files_properties(${SOURCE_FILE} PROPERTIES
        COMPILE_DEFINITIONS __FILE__="${SOURCE_FILE}"
      )
    endforeach(SOURCE_FILE)
    target_compile_options(${BIN_TARGET} PRIVATE -Wno-builtin-macro-redefined)
  endif()

  # Note: For Valgrind support.
  if(DEBUG_MODE)
    target_compile_options(${BIN_TARGET} PUBLIC -fno-omit-frame-pointer)
  endif()

  # Warnings for devilutionX
  target_compile_options(${BIN_TARGET} PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-char-subscripts -Wno-missing-braces)

  # For ARM and other default unsigned char platforms
  target_compile_options(${BIN_TARGET} PRIVATE -fsigned-char)

  # Disable Run-Time Type Information
  target_compile_options(${BIN_TARGET} PRIVATE -fno-rtti)
  # Disable Exceptions if requested
  if(DISABLE_EXCEPTIONS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    endif()
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  #target_compile_options(${BIN_TARGET} PRIVATE "/W1")
  target_compile_options(${BIN_TARGET} PRIVATE "-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING")
  target_compile_options(${BIN_TARGET} PRIVATE "/Zc:__cplusplus")
  # Disable Run-Time Type Information
  string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  target_compile_options(${BIN_TARGET} PRIVATE "/permissive-")
  # Disable Exceptions if requested
  if(DISABLE_EXCEPTIONS)
    string(REGEX REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_HAS_EXCEPTIONS=0")
  endif()
endif()

if(APPLE)
  set_source_files_properties("./Packaging/apple/AppIcon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER com.diasurgical.devilutionx)
  set(MACOSX_BUNDLE_COPYRIGHT Unlicense)
  set(MACOSX_BUNDLE_BUNDLE_NAME devilutionx)
  set(MACOSX_BUNDLE_DISPLAY_NAME DevilutionX)
  set(MACOSX_BUNDLE_INFO_STRING ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${PROJECT_VERSION}")
  if(IOS)
    set(MACOSX_BUNDLE_REQUIRED_PLATFORM IPhoneOS)
    set_target_properties(${BIN_TARGET} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "9.0")
  else()
    set(MACOSX_BUNDLE_REQUIRED_PLATFORM Carbon)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12.0")
  endif()
  set_target_properties(${BIN_TARGET} PROPERTIES MACOSX_BUNDLE_ICON_FILE "AppIcon.icns")
  set_target_properties(${BIN_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Packaging/apple/Info.plist")

  install (TARGETS ${BIN_TARGET} DESTINATION ./)

  if(MACOSX_STANDALONE_APP_BUNDLE)
      install(CODE "
        include(BundleUtilities)
        fixup_bundle(${CMAKE_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app \"\" \"\")
        "
        COMPONENT Runtime)
  endif()

  set(MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${PROJECT_VERSION}")
endif()

if(NINTENDO_SWITCH)
  set(APP_ROMFS       "${CMAKE_BINARY_DIR}/romfs")
  set(APP_ROMFS_FILES
    "${PROJECT_SOURCE_DIR}/Packaging/resources/devilx.mpq")

  add_custom_target(romfs_directory
    COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_ROMFS})

  add_custom_target(romfs_files
    COMMAND ${CMAKE_COMMAND} -E copy ${APP_ROMFS_FILES} ${APP_ROMFS}
    DEPENDS romfs_directory)

  # the romfs_directory target will create this directory at build time,
  # but the nx_create_nro function requires it to also exist at configure time
  file(MAKE_DIRECTORY ${APP_ROMFS})

  nx_generate_nacp (${BIN_TARGET}.nacp
    NAME    "DevilutionX"
    AUTHOR  "Devilution Team"
    VERSION "${PROJECT_VERSION}"
  )

  nx_create_nro(${BIN_TARGET}
    NACP ${BIN_TARGET}.nacp
    ICON "${PROJECT_SOURCE_DIR}/Packaging/switch/icon.jpg"
    ROMFS ${APP_ROMFS}
  )
  add_dependencies(${BIN_TARGET}_nro romfs_files)
endif()

if(VITA)
  set(VITA_APP_NAME "devilutionX")
  set(VITA_TITLEID  "DVLX00001")
  set(VITA_VERSION  "01.00")
  set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
  set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")
  vita_create_self(devilutionx.self devilutionx UNSAFE)
  vita_create_vpk(devilutionx.vpk ${VITA_TITLEID} devilutionx.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE Packaging/vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE Packaging/vita/sce_sys/pic0.png sce_sys/pic0.png
    FILE Packaging/vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE Packaging/vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE Packaging/vita/sce_sys/livearea/contents/logo0.png sce_sys/livearea/contents/logo0.png
    FILE Packaging/vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    FILE Packaging/resources/devilx.mpq devilx.mpq
  )
endif()

if(PS4)
  set(APP_MPQ_FOLDER  "${PROJECT_SOURCE_DIR}/Packaging/ps4")
  set(APP_MPQ_FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/devilx.mpq")
  add_custom_target(mpq_files
    COMMAND ${CMAKE_COMMAND} -E copy ${APP_MPQ_FILES} ${APP_MPQ_FOLDER})
  if(NOT PROJECT_VERSION)
    set(PROJECT_VERSION "01.00")
  endif()
  add_self(${BIN_TARGET})
  add_pkg(${BIN_TARGET} "${PROJECT_SOURCE_DIR}/Packaging/ps4"
    "DVLX00001" "DevilutionX" "${PROJECT_VERSION}")
  #get_filename_component(APP_TARGET_PREFIX ${BIN_TARGET} NAME_WE)
  #add_dependencies(${APP_TARGET_PREFIX}_pkg mpq_files)
  add_dependencies(${BIN_TARGET} mpq_files)
endif()

if(NINTENDO_3DS)
  set(APP_TITLE       "DevilutionX")
  set(APP_DESCRIPTION "DevilutionX port for 3DS")
  set(APP_AUTHOR      "Diasurgical Team")
  set(APP_ICON        "${PROJECT_SOURCE_DIR}/Packaging/ctr/icon.png")
  set(APP_BANNER      "${PROJECT_SOURCE_DIR}/Packaging/ctr/banner.png")
  set(APP_AUDIO       "${CMAKE_BINARY_DIR}/banner_audio.wav")
  set(APP_RSF         "${PROJECT_SOURCE_DIR}/Packaging/ctr/template.rsf")
  set(APP_ROMFS       "${CMAKE_BINARY_DIR}/romfs")
  set(APP_ROMFS_FILES
    "${PROJECT_SOURCE_DIR}/Packaging/resources/devilutionx.mpq"
    "${PROJECT_SOURCE_DIR}/Packaging/resources/devilx.mpq")
  if(HELLFIRE)
	  set(APP_TITLE       "DevilutionX: Hellfire")
	  set(APP_ICON        "${PROJECT_SOURCE_DIR}/Packaging/ctr/icon_hellfire.png")
	  set(APP_BANNER      "${PROJECT_SOURCE_DIR}/Packaging/ctr/banner_hellfire.png")
	  set(APP_RSF         "${PROJECT_SOURCE_DIR}/Packaging/ctr/template_hellfire.rsf")
  endif()
  set(APP_VERSION ${PROJECT_VERSION})

  find_program(FFMPEG ffmpeg)
  if(FFMPEG)
    add_custom_command(OUTPUT ${APP_AUDIO}
      COMMAND ${FFMPEG} -y -ss 3.3 -t 3 -i "${PROJECT_SOURCE_DIR}/Packaging/resources/shareware-startup.wav" -af "afade=t=in:st=0:d=0.1,afade=t=out:st=2.9:d=0.1" ${APP_AUDIO}
      DEPENDS ${PROJECT_SOURCE_DIR}/Packaging/resources/shareware-startup.wav
      VERBATIM)
  else()
    add_custom_command(OUTPUT ${APP_AUDIO}
      COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Packaging/ctr/audio_silent.wav ${APP_AUDIO}
      DEPENDS ${PROJECT_SOURCE_DIR}/Packaging/ctr/audio_silent.wav
      VERBATIM)
  endif()

  add_custom_target(romfs_directory
    COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_ROMFS})

  add_custom_target(romfs_files
    COMMAND ${CMAKE_COMMAND} -E copy ${APP_ROMFS_FILES} ${APP_ROMFS}
    DEPENDS romfs_directory)

  include(Tools3DS)
  add_3dsx_target(${BIN_TARGET})
  add_cia_target(${BIN_TARGET} ${APP_RSF} ${APP_BANNER} ${APP_AUDIO})

  get_filename_component(APP_TARGET_PREFIX ${BIN_TARGET} NAME_WE)
  add_dependencies(${APP_TARGET_PREFIX}_3dsx romfs_files)
  add_dependencies(${APP_TARGET_PREFIX}_cia romfs_files)
endif()
message(STATUS "output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} lib: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} bin: ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
if(CPACK)
  if(WIN32)
    set(CPACK_PACKAGE_FILE_NAME "devilutionx")
    set(CPACK_PACKAGE_NAME ${project_name})
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_STRIP_FILES TRUE)
    install(TARGETS ${BIN_TARGET} DESTINATION .)
    #install(DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/"
    #  DESTINATION "."
    #  PATTERN "*.lib" EXCLUDE
    #  PATTERN "*.exp" EXCLUDE
    #  PATTERN "*.h" EXCLUDE
    #)
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/windows/README.txt"
      DESTINATION "."
    )
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/devilx.mpq"
      DESTINATION "."
    )
    #install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/LICENSE.CharisSILB.txt"
    #  DESTINATION "LICENSE"
    #)
    #install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/LICENSE.freetype.txt"
    #  DESTINATION "LICENSE"
    #)
    if(ZEROTIER)
      install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/LICENSE.zlib.txt"
        DESTINATION "LICENSE"
      )
    endif()
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/README-SDL.txt"
      DESTINATION "LICENSE"
    )
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/brotlidec.dll"
    #  DESTINATION "."
    #)
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bz2.dll"
    #  DESTINATION "."
    #)
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype.dll"
    #  DESTINATION "."
    #)
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libpng16.dll"
    #  DESTINATION "."
    #)
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/SDL2.dll"
    #  DESTINATION "."
    #)
    #install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlib1.dll"
    #  DESTINATION "."
    #)
    #if(NOT NONET AND DEVILUTIONX_STATIC_LIBSODIUM)
    #  install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libsodium.dll"
    #    DESTINATION "."
    #  )
    #endif()
    ##if(SYSTEM_BITS MATCHES x64)
    #if(${CMAKE_SIZEOF_VOID_P} EQUAL "8")
    #  #install(FILES "${PROJECT_SOURCE_DIR}/SDL2-2.0.14/x86_64-w64-mingw32/bin/SDL2.dll"
    #  install(FILES "${PROJECT_SOURCE_DIR}/vcpkg/packages/sdl2_x64-windows/bin/SDL2.dll"
    #    DESTINATION "."
    #  )
    #  #install(FILES "${PROJECT_SOURCE_DIR}/SDL2-2.0.14/README-SDL.txt"
    #  #  DESTINATION "LICENSE"
    #  #)
    #else()
    #  #install(FILES "${PROJECT_SOURCE_DIR}/SDL2-2.0.14/x86_64-w64-mingw32/bin/SDL2.dll"
    #  install(FILES "${PROJECT_SOURCE_DIR}/vcpkg/packages/sdl2_x86-windows/bin/SDL2.dll"
    #    DESTINATION "."
    #  )
    #  #install(FILES "${PROJECT_SOURCE_DIR}/SDL2-2.0.14/README-SDL.txt"
    #  #  DESTINATION "LICENSE"
    #  #)
    #endif()

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    string(TOLOWER ${PROJECT_NAME} project_name)
    set(CPACK_PACKAGE_NAME ${project_name})
    set(CPACK_GENERATOR "7Z")
    # Common *nix files
    set(CPACK_STRIP_FILES TRUE)
    install(TARGETS ${BIN_TARGET} DESTINATION bin)
    set(desktop_file "${PROJECT_SOURCE_DIR}/Packaging/nix/${project_name}.desktop")
    set(desktop_file_hellfire "${PROJECT_SOURCE_DIR}/Packaging/nix/${project_name}-hellfire.desktop")

    find_program(DFI desktop-file-install)
    if(DFI)
      execute_process(COMMAND ${DFI} --dir=${CMAKE_BINARY_DIR} ${desktop_file})
      set(desktop_file "${CMAKE_BINARY_DIR}/${project_name}.desktop")
      execute_process(COMMAND ${DFI} --dir=${CMAKE_BINARY_DIR} ${desktop_file_hellfire})
      set(desktop_file_hellfire "${CMAKE_BINARY_DIR}/${project_name}-hellfire.desktop")
    endif()

    install(FILES "${desktop_file}"
      DESTINATION "share/applications"
    )
    install(FILES "${desktop_file_hellfire}"
      DESTINATION "share/applications"
    )
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/nix/README.txt"
      DESTINATION "share/diasurgical/${project_name}"
    )
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/devilx.mpq"
      DESTINATION "share/diasurgical/${project_name}"
    )
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/icon.png"
      DESTINATION "share/icons/hicolor/512x512/apps"
      RENAME "${project_name}.png"
    )
    install(FILES "${PROJECT_SOURCE_DIR}/Packaging/resources/hellfire.png"
      DESTINATION "share/icons/hicolor/512x512/apps"
      RENAME "${project_name}-hellfire.png"
    )

    # -G DEB
    #set(CPACK_PACKAGE_CONTACT "anders@jenbo.dk")
    if(USE_SDL1)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl1.2debian")
    else()
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0")
    endif()
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    # -G RPM
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

    find_program(RPMBUILD rpmbuild)
    if(RPMBUILD)
      list(APPEND CPACK_GENERATOR "RPM")
    endif()
    find_program(DPKG dpkg)
    if(DPKG)
      list(APPEND CPACK_GENERATOR "DEB")
    endif()

  elseif(APPLE)
    set(CPACK_PACKAGE_FILE_NAME "devilutionx")
    set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK "ON")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_GENERATOR "DragNDrop")
  endif()

  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
  include(CPack)
endif()

##### Info output #####
set(LONGESTOPTIONNAME 31)

message(STATUS "")
message(STATUS "Devilx was configured with the following options:")
message(STATUS "")
message(STATUS "Platform: ${CMAKE_SYSTEM}")
message(STATUS "Target platform: ${TARGET_PLATFORM}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER} .. (${CMAKE_CXX_COMPILER_ID})")
message(STATUS "Revision: ${VERSION_NUM}")
message(STATUS "")
message(STATUS "Defines:")
# Defines without value
foreach(
  def_name
  HELLFIRE
  NOSOUND
  STREAM_ALL_AUDIO
  NOWIDESCREEN
  NONET
  INET_MODE
  NETENCRYPT
  ADAPTIVE_NETUPDATE
  NOHOSTING
  HOSTONLY
  TCPIP
  ZEROTIER
  HAS_GAMECTRL
  HAS_DPAD
  HAS_TOUCHPAD
  HAS_KBCTRL
  HAS_JOYSTICK
  PREFILL_PLAYER_NAME
  ASAN
  UBSAN
  TSAN
  DEBUG_MODE
  DEV_MODE
  RUN_TESTS
  DISABLE_EXCEPTIONS
  DISABLE_LTO
  PIE
  CPACK
  USE_LD_MOLD
  MACOSX_STANDALONE_APP_BUNDLE
  GPERF
  GPERF_HEAP_FIRST_GAME_ITERATION
)
  # Get the padding
  string(LENGTH ${def_name} _OPTLEN)
  math(EXPR _PADLEN "(${LONGESTOPTIONNAME} + 1) - ${_OPTLEN}")
  string(RANDOM LENGTH ${_PADLEN} ALPHABET " " _PADDING)
  if(${def_name})
    set(HAVE_${def_name} ON)
  else()
    set(HAVE_${def_name} OFF)
  endif()
  message(STATUS "  ${def_name}${_PADDING}: ${HAVE_${def_name}}")
endforeach(def_name)
# Defines with value
foreach(
  def_name
  ASSET_MPL
  DEFAULT_WIDTH
  DEFAULT_HEIGHT
  SCREEN_WIDTH
  SCREEN_HEIGHT
  #TTF_FONT_DIR
  #TTF_FONT_NAME
  SDL1_VIDEO_MODE_BPP
  SDL1_VIDEO_MODE_FLAGS
  SDL1_VIDEO_MODE_SVID_FLAGS
  SDL1_FORCE_SVID_VIDEO_MODE
  KBCTRL_BUTTON_DPAD_LEFT
  KBCTRL_BUTTON_DPAD_RIGHT
  KBCTRL_BUTTON_DPAD_UP
  KBCTRL_BUTTON_DPAD_DOWN
  KBCTRL_BUTTON_B
  KBCTRL_BUTTON_A
  KBCTRL_BUTTON_Y
  KBCTRL_BUTTON_X
  KBCTRL_BUTTON_LEFTSTICK
  KBCTRL_BUTTON_RIGHTSTICK
  KBCTRL_BUTTON_RIGHTSHOULDER
  KBCTRL_BUTTON_LEFTSHOULDER
  KBCTRL_BUTTON_TRIGGERLEFT
  KBCTRL_BUTTON_TRIGGERRIGHT
  KBCTRL_BUTTON_START
  KBCTRL_BUTTON_BACK
  KBCTRL_IGNORE_1
  JOY_AXIS_LEFTX
  JOY_AXIS_LEFTY
  JOY_AXIS_RIGHTX
  JOY_AXIS_RIGHTY
  JOY_HAT_DPAD_UP_HAT
  JOY_HAT_DPAD_UP
  JOY_HAT_DPAD_DOWN_HAT
  JOY_HAT_DPAD_DOWN
  JOY_HAT_DPAD_LEFT_HAT
  JOY_HAT_DPAD_LEFT
  JOY_HAT_DPAD_RIGHT_HAT
  JOY_HAT_DPAD_RIGHT
  JOY_BUTTON_DPAD_LEFT
  JOY_BUTTON_DPAD_RIGHT
  JOY_BUTTON_DPAD_UP
  JOY_BUTTON_DPAD_DOWN
  JOY_BUTTON_B
  JOY_BUTTON_A
  JOY_BUTTON_Y
  JOY_BUTTON_X
  JOY_BUTTON_LEFTSTICK
  JOY_BUTTON_RIGHTSTICK
  JOY_BUTTON_RIGHTSHOULDER
  JOY_BUTTON_LEFTSHOULDER
  JOY_BUTTON_TRIGGERLEFT
  JOY_BUTTON_TRIGGERRIGHT
  JOY_BUTTON_START
  JOY_BUTTON_BACK
  REMAP_KEYBOARD_KEYS
)
  string(LENGTH ${def_name} _OPTLEN)
  math(EXPR _PADLEN "(${LONGESTOPTIONNAME} + 1) - ${_OPTLEN}")
  string(RANDOM LENGTH ${_PADLEN} ALPHABET " " _PADDING)
  if(DEFINED ${def_name})
    set(VALUEOF_${def_name} ${${def_name}})
  else()
    set(VALUEOF_${def_name} "__UNDEFINED__")
  endif()
  message(STATUS "  ${def_name}${_PADDING}: ${VALUEOF_${def_name}}")
endforeach(def_name)
message(STATUS "")
message(STATUS " CFLAGS:        ${CMAKE_C_FLAGS}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  message(STATUS " CMAKE_C_FLAGS_DEBUG:   ${CMAKE_C_FLAGS_DEBUG}")
  message(STATUS " CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
endif()
message(STATUS "")
if(USE_SDL1)
  message(STATUS " Using dynamically linked SDL1.")
else()
  if(DEVILUTIONX_SYSTEM_SDL2)
    message(STATUS " Using system SDL2.")
  elseif(DEVILUTIONX_STATIC_SDL2)
    message(STATUS " Using statically linked SDL2.")
  else()
    message(STATUS " Using dynamically linked SDL2.")
  endif()
endif()
if(DEVILUTIONX_STATIC_LIBSODIUM)
  set(_LINK_MODE "statically")
else()
  set(_LINK_MODE "dynamically")
endif()
if(DEVILUTIONX_SYSTEM_LIBSODIUM)
  set(_SOURCE "system ")
else()
  set(_SOURCE "")
endif()
message(STATUS " Using ${_LINK_MODE} linked ${_SOURCE}libsodium.")
message(STATUS "")
