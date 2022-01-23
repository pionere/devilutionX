/**
 * @file appfat.cpp
 *
 * Implementation of error dialogs.
 */
#include "all.h"
#include <config.h>
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Displays an error message box based on the given format string and variable argument list.
 * @param pszFmt Error message format
 * @param va Additional parameters for message format
 */
static void MsgBox(const char *pszFmt, va_list va)
{
	char text[256];

	vsnprintf(text, sizeof(text), pszFmt, va);

	UiErrorOkDialog("Error", text);
}

/**
 * @brief Cleans up after a fatal application error.
 */
static void FreeDlg()
{
	NetClose();
}

void app_fatal(const char *pszFmt, ...)
{
	va_list va;

	va_start(va, pszFmt);
	FreeDlg();

	if (pszFmt != NULL)
		MsgBox(pszFmt, va);

	va_end(va);

	diablo_quit(1);
}

void app_warn(const char *pszFmt, ...)
{
	char text[256];
	va_list va;

	va_start(va, pszFmt);
	vsnprintf(text, sizeof(text), pszFmt, va);
	va_end(va);

	UiErrorOkDialog(PROJECT_NAME, text, false);
}

#if DEBUG_MODE
/**
 * @brief Show an error and exit the application.
 * @param nLineNo The line number of the assertion
 * @param pszFile File name where the assertion is located
 * @param pszFail Fail message
 */
void assert_fail(int nLineNo, const char *pszFile, const char *pszFail)
{
	app_fatal("assertion failed (%d:%s)\n%s", nLineNo, pszFile, pszFail);
}

/**
 * @brief Terminates the game and displays an error dialog box based on the given dialog_id.
 */
void ErrDlg(const char *title, const char *error, const char *log_file_path, int log_line_nr)
{
	char text[1024];

	FreeDlg();

	snprintf(text, sizeof(text), "%s\n\nThe error occurred at: %s line %d", error, log_file_path, log_line_nr);

	UiErrorOkDialog(title, text);
	app_fatal(NULL);
}
#endif

/**
 * @brief Terminates the game with a file not found error dialog.
 */
/*void FileErrDlg(const char *error)
{
	char text[1024];

	FreeDlg();

	if (error == NULL)
		error = "";
	snprintf(
	    text,
	    sizeof(text),
	    "Unable to open a required file.\n"
	    "\n"
	    "Verify that the MD5 of diabdat.mpq matches one of the following values\n"
	    "011bc6518e6166206231080a4440b373\n"
	    "68f049866b44688a7af65ba766bef75a\n"
	    "\n"
	    "The problem occurred when loading:\n%s",
	    error);

	UiErrorOkDialog("Data File Error", text);
	app_fatal(NULL);
}*/

/**
 * @brief Terminates the game with an insert CD error dialog.
 */
/*void InsertCDDlg()
{
	char text[1024];
	snprintf(
	    text,
	    sizeof(text),
	    "Unable to open %s.\n"
	    "\n"
	    "Make sure that it is in the game folder.",
	    DATA_ARCHIVE_MAIN);

	UiErrorOkDialog("Data File Error", text);
	app_fatal(NULL);
}*/

DEVILUTION_END_NAMESPACE
