/**
 * @file init.cpp
 *
 * Implementation of routines for initializing the environment, disable screen saver, load MPQ.
 */
#include <string>

#include "all.h"
#include "paths.h"
#include <SDL.h>
#include <config.h>

DEVILUTION_BEGIN_NAMESPACE

_SNETVERSIONDATA fileinfo;
/** True if the game is the current active window */
int gbActive;
/** The current input handler function */
WNDPROC CurrentProc;
/** A handle to the mpq archives. */
HANDLE diabdat_mpqs[NUM_MPQS];

namespace {

HANDLE init_test_access(const char *mpq_name, const char *reg_loc, int dwPriority, int fs)
{
	HANDLE archive;
	const std::string *paths[2] = { &GetBasePath(), &GetPrefPath() };
	std::string mpq_abspath;
	DWORD mpq_flags = 0;
#if !defined(__SWITCH__) && !defined(__AMIGA__)
	mpq_flags |= MPQ_FLAG_READ_ONLY;
#endif
	for (int i = 0; i < 2; i++) {
		mpq_abspath = *paths[i] + mpq_name;
		if (SFileOpenArchive(mpq_abspath.c_str(), dwPriority, mpq_flags, &archive)) {
			SFileSetBasePath(paths[i]->c_str());
			return archive;
		}
	}

	return NULL;
}

} // namespace

/* data */

char gszVersionNumber[MAX_SEND_STR_LEN] = "internal version unknown";
char gszProductName[MAX_SEND_STR_LEN] = "Diablo v1.09";

void init_cleanup()
{
	int i;

	pfile_flush_W();

	for (i = 0; i < NUM_MPQS; i++) {
		if (diabdat_mpqs[i] != NULL) {
			SFileCloseArchive(diabdat_mpqs[i]);
			diabdat_mpqs[i] = NULL;
		}
	}

	NetClose();
}

static void init_get_file_info()
{
	snprintf(gszProductName, sizeof(gszProductName), "%s v%s", PROJECT_NAME, PROJECT_VERSION);
	snprintf(gszVersionNumber, sizeof(gszVersionNumber), "version %s", PROJECT_VERSION);
}

void init_archives()
{
	HANDLE fh = NULL;
	memset(&fileinfo, 0, sizeof(fileinfo));
	fileinfo.size = sizeof(fileinfo);
	fileinfo.versionstring = gszVersionNumber;
	fileinfo.executablefile = "";
	fileinfo.originalarchivefile = "";
	fileinfo.patcharchivefile = "";
	init_get_file_info();

#ifdef SPAWN
	diabdat_mpqs[MPQ_DIABDAT] = init_test_access("spawn.mpq", "DiabloSpawn", 1000, FS_PC);
	diabdat_mpqs[MPQ_PATCH_RT] = init_test_access("patch_sh.mpq", "DiabloSpawn", 2000, FS_PC);
#else
	diabdat_mpqs[MPQ_DIABDAT] = init_test_access("diabdat.mpq", "DiabloCD", 1000, FS_CD);
	diabdat_mpqs[MPQ_PATCH_RT] = init_test_access("patch_rt.mpq", "DiabloInstall", 2000, FS_PC);
#endif
	if (!SFileOpenFile("ui_art\\title.pcx", &fh))
		InsertCDDlg();
	SFileCloseFile(fh);

#ifdef HELLFIRE
	diabdat_mpqs[MPQ_HELLFIRE] = init_test_access("hellfire.mpq", "DiabloInstall", 8000, FS_PC);
	diabdat_mpqs[MPQ_HF_MONK] = init_test_access("hfmonk.mpq", "DiabloInstall", 8100, FS_PC);
	diabdat_mpqs[MPQ_HF_BARD] = init_test_access("hfbard.mpq", "DiabloInstall", 8110, FS_PC);
	diabdat_mpqs[MPQ_HF_BARB] = init_test_access("hfbarb.mpq", "DiabloInstall", 8120, FS_PC);
	diabdat_mpqs[MPQ_HF_MUSIC] = init_test_access("hfmusic.mpq", "DiabloInstall", 8200, FS_PC);
	diabdat_mpqs[MPQ_HF_VOICE] = init_test_access("hfvoice.mpq", "DiabloInstall", 8500, FS_PC);
	diabdat_mpqs[MPQ_HF_OPT1] = init_test_access("hfopt1.mpq", "DiabloInstall", 8600, FS_PC);
	diabdat_mpqs[MPQ_HF_OPT2] = init_test_access("hfopt2.mpq", "DiabloInstall", 8610, FS_PC);
	diabdat_mpqs[MPQ_DEVILUTIONX] = init_test_access("devilutionx.mpq", "DiabloInstall", 9000, FS_PC);
#endif
}

void init_create_window()
{
	if (!SpawnWindow(PROJECT_NAME))
		app_fatal("Unable to create main window");
	dx_init(NULL);
	gbActive = true;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * SCREEN_Y];
	gpBufEnd = (BYTE *)(BUFFER_WIDTH * (SCREEN_HEIGHT + SCREEN_Y));
	SDL_DisableScreenSaver();
}

void MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case DVL_WM_PAINT:
		gbRedrawFlags = REDRAW_ALL;
		break;
	case DVL_WM_QUERYENDSESSION:
		diablo_quit(0);
		break;
	}
}

WNDPROC SetWindowProc(WNDPROC NewProc)
{
	WNDPROC OldProc;

	OldProc = CurrentProc;
	CurrentProc = NewProc;
	return OldProc;
}

DEVILUTION_END_NAMESPACE
