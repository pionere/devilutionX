SET(MINGW_CROSS TRUE)

SET(CROSS_PREFIX "/usr" CACHE STRING "crosstool-NG prefix")

SET(CMAKE_SYSTEM_NAME Windows)

# workaround
list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "${CROSS_PREFIX}/x86_64-w64-mingw32/include")

SET(CMAKE_C_COMPILER   "x86_64-w64-mingw32-gcc")
SET(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
set(CMAKE_RC_COMPILER "x86_64-w64-mingw32-windres")
set(PKG_CONFIG_EXECUTABLE "${CROSS_PREFIX}/bin/x86_64-w64-mingw32-pkg-config" CACHE STRING "Path to pkg-config")

SET(CMAKE_FIND_ROOT_PATH  "${CROSS_PREFIX}/x86_64-w64-mingw32" "${CROSS_PREFIX}/x86_64-w64-mingw32/x86_64-w64-mingw32")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(SYSTEM_BITS x64)
