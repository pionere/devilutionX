include(FetchContent_MakeAvailableExcludeFromAll)

set(BUILD_HOST_SELFTEST FALSE)
set(BUILD_STATIC_LIB TRUE)
set(BUILD_SHARED_LIB FALSE)
set(ALLOW_INSTALL_TARGET FALSE)
set(OMIT_JSON_SUPPORT TRUE)

include(FetchContent)
FetchContent_Declare(libzt
  #GIT_REPOSITORY https://github.com/diasurgical/libzt.git
  #GIT_TAG db7b642a4ce9f0f5e0ba7f293bd7ffa7897e4831)
  GIT_REPOSITORY https://github.com/pionere/libzt.git
  GIT_TAG 70ea658a8faa878e2f6545d1cea2d590622946cc)
FetchContent_MakeAvailableExcludeFromAll(libzt)

#if(NOT ANDROID)
#  set(libzt_LIB_NAME zt-static)
#  #set(libzt_LIB_NAME zt-shared)
#else()
#  set(libzt_LIB_NAME zt-shared)
#endif()
if(TARGET zt-static)
  set(libzt_LIB_NAME zt-static)
else()
  set(libzt_LIB_NAME zt-shared)
endif()

foreach(
  lib_name
  zto_obj
  libnatpmp_obj
  libzt_obj
  lwip_obj
  miniupnpc_obj
  zto_pic
  zt_pic
  natpmp_pic
  lwip_pic
  miniupnpc_pic
  ztcore
  ${libzt_LIB_NAME}
)
  if(TARGET ${lib_name})
    # External library, ignore all warnings
    #target_compile_options(${lib_name} PRIVATE -w)
  endif()
endforeach(lib_name)

target_include_directories(${libzt_LIB_NAME} INTERFACE
  "${libzt_SOURCE_DIR}/include"
  "${libzt_SOURCE_DIR}/src"
  "${libzt_SOURCE_DIR}/ext/lwip/src/include")

if(WIN32)
  # the code below might be necessary to build when SDL_LIBC is OFF.
  #  See https://github.com/libsdl-org/SDL/issues/5156 .
  #target_compile_definitions(natpmp_pic PRIVATE WIN32)
  #target_compile_definitions(libnatpmp_obj PRIVATE WIN32)
  #target_compile_definitions(${libzt_LIB_NAME} PRIVATE WIN32)
  target_include_directories(${libzt_LIB_NAME} INTERFACE
    "${libzt_SOURCE_DIR}/ext/lwip-contrib/ports/win32/include")
else()
  target_include_directories(${libzt_LIB_NAME} INTERFACE
    "${libzt_SOURCE_DIR}/ext/lwip-contrib/ports/unix/port/include")
endif()

if(MINGW_CROSS)
  option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)

  FetchContent_Declare(mingw-std-threads
    #GIT_REPOSITORY https://github.com/meganz/mingw-std-threads
    #GIT_TAG bee085c0a6cb32c59f0b55c7bba976fe6dcfca7f
    GIT_REPOSITORY https://github.com/pionere/mingw-std-threads
    GIT_TAG fc4127c956734f6a18f6fa3236d19484ce97f8d9
  )
  FetchContent_MakeAvailableExcludeFromAll(mingw-std-threads)

  target_compile_definitions(libnatpmp_obj PRIVATE -D_WIN32_WINNT=0x601 -DSTATICLIB)
  target_compile_definitions(zto_obj PRIVATE -D_WIN32_WINNT=0x601)
  target_compile_definitions(zto_pic PRIVATE -D_WIN32_WINNT=0x601)
  target_compile_definitions(libzt_obj PRIVATE -D_WIN32_WINNT=0x601)
  target_compile_definitions(zt_pic PRIVATE -D_WIN32_WINNT=0x601)
  target_compile_definitions(${libzt_LIB_NAME} PRIVATE -D_WIN32_WINNT=0x601 -DADD_EXPORTS=1)
  target_compile_definitions(${libzt_LIB_NAME} PUBLIC -DADD_EXPORTS=1)
  target_link_libraries(libzt_obj PRIVATE mingw_stdthreads)
  target_link_libraries(${libzt_LIB_NAME} mingw_stdthreads)
endif()

#if(MSVC AND TARGET libnatpmp_obj)
#  target_compile_definitions(libnatpmp_obj PRIVATE -DSTATICLIB)
#endif()

add_library(zt ALIAS ${libzt_LIB_NAME})
