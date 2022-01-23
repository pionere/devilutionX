/**
 * @file appfat.h
 *
 * Interface of error dialogs.
 */
#ifndef __APPFAT_H__
#define __APPFAT_H__

#include "../types.h"

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
#ifdef DEBUG_MODE
void assert_fail(int nLineNo, const char *pszFile, const char *pszFail);
void ErrDlg(const char *title, const char *error, const char *log_file_path, int log_line_nr);
#endif
//void FileErrDlg(const char *error);
//void InsertCDDlg();

#if _DEVMODE || DEBUG_MODE
/*template <typename... MsgArgs>
void dev_fatal(const char *pszFmt, MsgArgs... args) {
	app_fatal(pszFmt, args...);
}*/
#define dev_fatal(msg, ...) app_fatal(msg, __VA_ARGS__);
#if DEBUG_MODE
#define sdl_fatal(ec) \
	if (ec == ec) { \
		ErrDlg("SDL Error", SDL_GetError(), __FILE__, __LINE__); \
	}
#define ttf_fatal(ec) \
	if (ec == ec) { \
		ErrDlg("SDL Error", TTF_GetError(), __FILE__, __LINE__); \
	}
#else
#define sdl_fatal(error_code) app_fatal("SDL Error %d:%s", error_code, SDL_GetError())
#endif // DEBUG_MODE
#else
inline void dev_fatal(const char *pszFmt, ...) {}
#define sdl_fatal(error_code) app_fatal("SDL Error %d:%s", error_code, SDL_GetError())
#endif // _DEVMODE || DEBUG_MODE

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __APPFAT_H__ */
