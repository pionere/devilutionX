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

void TriggerBreak();
#define TraceLastError SDL_GetError
void app_fatal(const char *pszFmt, ...); //  DVL_PRINTF_ATTRIBUTE(1, 2);
void DrawDlg(const char *pszFmt, ...); // DVL_PRINTF_ATTRIBUTE(1, 2);
#ifdef _DEBUG
void assert_fail(int nLineNo, const char *pszFile, const char *pszFail);
#endif
void DDErrMsg(DWORD error_code, int log_line_nr, const char *log_file_path);
void DSErrMsg(DWORD error_code, int log_line_nr, const char *log_file_path);
void ErrDlg(const char *title, const char *error, const char *log_file_path, int log_line_nr);
void FileErrDlg(const char *error);
void DiskFreeDlg(const char *error);
void InsertCDDlg();
void DirErrorDlg(const char *error);

#ifdef __cplusplus
}
#endif

#if defined(_DEVMODE) || defined(_DEBUG)
template <typename... MsgArgs>
void dev_fatal(const char *pszFmt, MsgArgs... args) {
	app_fatal(pszFmt, args...);
}
#else
inline void dev_fatal(const char *pszFmt, ...) {}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __APPFAT_H__ */
