/**
 * @file appfat.h
 *
 * Interface of error dialogs.
 */
#ifndef __APPFAT_H__
#define __APPFAT_H__

#include "../types.h"
#include "utils/log.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Terminates the game and displays an error message box.
 * @param pszFmt Optional error message.
 * @param ... (see printf)
 */
void app_fatal(const char *pszFmt, ...); // DVL_PRINTF_ATTRIBUTE(1, 2);

/**
 * @brief Displays a warning message box based on the given formatted error message.
 * @param pszFmt Error message format
 * @param ... Additional parameters for message format
 */
void app_warn(const char *pszFmt, ...); // DVL_PRINTF_ATTRIBUTE(1, 2);
#if DEBUG_MODE
void ErrDlg(const char *title, const char *error, const char *log_file_path, int log_line_nr);
#endif
//void FileErrDlg(const char *error);
//void InsertCDDlg();

#if DEBUG_MODE || DEV_MODE
/*template <typename... MsgArgs>
void dev_fatal(const char *pszFmt, MsgArgs... args) {
	app_fatal(pszFmt, args...);
}*/
#define dev_fatal(msg, ...) app_fatal(msg, __VA_ARGS__);
#if DEBUG_MODE
#define app_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(app.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("App Error %d @ %s:%d", ec, __FILE__, __LINE__); \
	}
#define sdl_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(sdl.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("SDL Error %d: '%s' @ %s:%d", ec, SDL_GetError(), __FILE__, __LINE__); \
	}
#else
#define app_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(sdl.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("App Error %d", ec); \
	}
#define sdl_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(sdl.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("SDL Error %d", ec); \
	}
#endif // DEBUG_MODE
#else
#define dev_fatal(msg, ...) do { } while(0);
#define app_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(sdl.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("App Error %d", ec); \
	}
#define sdl_error(ec) \
	if (ec == ec) { \
		DoLog("ABORT(sdl.%d): %s @ %s:%d", ec, __FUNCTION__, __FILE__, __LINE__); \
		app_fatal("SDL Error %d", ec); \
	}
#endif // DEBUG_MODE || DEV_MODE

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __APPFAT_H__ */
