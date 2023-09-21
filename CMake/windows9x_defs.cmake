#set(ASAN OFF)
#set(UBSAN OFF)
set(USE_PATCH OFF)

#set(NONET ON)
#set(DISABLE_ZERO_TIER ON)
set(USE_SDL1 ON)
#set(DEVILUTIONX_SYSTEM_BZIP2 OFF)
#set(DEVILUTIONX_SYSTEM_LIBFMT OFF)
#set(DEVILUTIONX_STATIC_LIBSODIUM OFF)

# Compatibility with Windows 9x 8-bit mode and improved performance
set(SDL1_VIDEO_MODE_BPP 8)
#set(SDL1_FORCE_DIRECT_RENDER ON)

#set(DEVILUTIONX_WINDOWS_NO_WCHAR ON)

# `WINVER=0x0500` without `_WIN32_WINNT` is Windows 98.
# MinGW force-defines `_WIN32_WINNT=0xa00` if it isn't defined, so define it as 0.
#add_definitions(-DWINVER=0x0500 -D_WIN32_WINDOWS=0x0500 -D_WIN32_WINNT=0)
set(WINVER 0x0500)
set(_WIN32_WINNT 0x0300)

#list(APPEND DEVILUTIONX_PLATFORM_LINK_LIBRARIES
#  shlwapi
#  wsock32
#  ws2_32
#  wininet
#)

#if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#  list(APPEND DEVILUTIONX_PLATFORM_COMPILE_OPTIONS "/W3" "/Zc:__cplusplus" "/utf-8")
#  list(APPEND DEVILUTIONX_PLATFORM_COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
#else()
#  list(APPEND DEVILUTIONX_PLATFORM_COMPILE_OPTIONS $<$<CONFIG:Debug>:-gstabs>)
#endif()

#if(MINGW_CROSS)
#  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/mingw9x")
#endif()
