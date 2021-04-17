/**
 * @file init.cpp
 *
 * Implementation of routines for initializing the environment, disable screen saver, load MPQ.
 */
#include <string>

#include "all.h"
#include <config.h>
#include "utils/paths.h"
#include <SDL.h>
#include <string>
#include <fstream>
#ifdef _DEVMODE
#include <sys/stat.h>
#endif

#ifdef __vita__
// increase default allowed heap size on Vita
int _newlib_heap_size_user = 100 * 1024 * 1024;
#endif

DEVILUTION_BEGIN_NAMESPACE

/** True if the game is the current active window */
bool gbActive;
/** The current input handler function */
WNDPROC CurrentWndProc;
/** A handle to the mpq archives. */
#ifdef MPQONE
HANDLE diabdat_mpq;
#else
HANDLE diabdat_mpqs[NUM_MPQS];
#endif

namespace {

HANDLE init_test_access(const char *mpq_name)
{
	HANDLE archive;
	const char *paths[2] = { GetBasePath(), GetPrefPath() };
	std::string mpq_abspath;
	DWORD mpq_flags = 0;
#if !defined(__SWITCH__) && !defined(__AMIGA__) && !defined(__vita__)
	mpq_flags |= MPQ_FLAG_READ_ONLY;
#endif
	for (int i = 0; i < 2; i++) {
		mpq_abspath = paths[i];
		mpq_abspath += mpq_name;
		if (SFileOpenArchive(mpq_abspath.c_str(), mpq_flags, &archive)) {
			SFileSetBasePath(paths[i]);
			return archive;
		}
	}

	return NULL;
}

} // namespace

/* data */

char gszProductName[MAX_SEND_STR_LEN] = "Diablo v1.09";

void init_cleanup()
{
	pfile_flush_W();

#ifdef MPQONE
	SFileCloseArchive(diabdat_mpq);
	diabdat_mpq = NULL;
#else
	int i;
	for (i = 0; i < NUM_MPQS; i++) {
		if (diabdat_mpqs[i] != NULL) {
			SFileCloseArchive(diabdat_mpqs[i]);
			diabdat_mpqs[i] = NULL;
		}
	}
#endif
}

static void init_get_file_info()
{
	snprintf(gszProductName, sizeof(gszProductName), "%s v%s", PROJECT_NAME, PROJECT_VERSION);
}

#ifdef _DEVMODE
static void CreateMpq(const char* destMpqName, const char* folder, const char *files)
{
	std::string basePath = std::string(GetBasePath()) + folder;
	std::ifstream input(std::string(GetBasePath()) + files);

	int entryCount = 0;
	std::string line;
	while (std::getline(input, line)) {
		std::string path = basePath + line.c_str();
		FILE *fp = fopen(path.c_str(), "r");
		if (fp != NULL) {
			fclose(fp);
			entryCount++;
		}
	}
	input.close();
	// TODO: use GetNearestPowerOfTwo of StormCommon.h?
	int hashCount = 1;
	while (hashCount < entryCount) {
		hashCount <<= 1;
	}
	
	std::string path = std::string(GetBasePath()) + destMpqName;
	if (!OpenMPQ(path.c_str(), hashCount, hashCount))
		app_fatal("Unable to open MPQ file %s.", path.c_str());
	
	input = std::ifstream(std::string(GetBasePath()) + files);
	while (std::getline(input, line)) {
		std::string path = basePath + line.c_str();
		FILE *fp = fopen(path.c_str(), "rb");
		if (fp != NULL) {
			struct stat st;
			stat(path.c_str(), &st);
			BYTE* buf = DiabloAllocPtr(st.st_size);
			int readBytes = fread(buf, 1, st.st_size, fp);
			fclose(fp);
			if (!mpqapi_write_file(line.c_str(), buf, st.st_size))
				app_fatal("Unable to write %s to the MPQ.", line.c_str());
			mem_free_dbg(buf);
		}
	}
	input.close();
	mpqapi_flush_and_close(true);
}
#endif

void init_archives()
{
	init_get_file_info();

	InitializeMpqCryptography();
	SFileEnableDirectAccess(getIniBool("devilutionx", "Direct FileAccess", false));

	//CreateMpq("devilx.mpq", "Work\\", "mpqfiles.txt");
#ifdef MPQONE
	diabdat_mpq = init_test_access(MPQONE);
	if (diabdat_mpq != NULL)
		return;

	HANDLE diabdat_mpqs[NUM_MPQS];
#endif
	diabdat_mpqs[MPQ_DIABDAT] = init_test_access(DATA_ARCHIVE_MAIN);
	if (diabdat_mpqs[MPQ_DIABDAT] == NULL)
		diabdat_mpqs[MPQ_DIABDAT] = init_test_access(DATA_ARCHIVE_MAIN_ALT);
	if (diabdat_mpqs[MPQ_DIABDAT] == NULL)
		app_fatal("Can not find/access '%s' in the game folder.", DATA_ARCHIVE_MAIN);
	diabdat_mpqs[MPQ_PATCH_RT] = init_test_access(DATA_ARCHIVE_PATCH);
	//if (!SFileOpenFileEx(diabdat_mpqs[MPQ_DIABDAT], "ui_art\\title.pcx", SFILE_OPEN_CHECK_EXISTS, NULL))
	//	InsertCDDlg();

#ifdef HELLFIRE
	diabdat_mpqs[MPQ_HELLFIRE] = init_test_access("hellfire.mpq");
	diabdat_mpqs[MPQ_HF_MONK] = init_test_access("hfmonk.mpq");
	diabdat_mpqs[MPQ_HF_BARD] = init_test_access("hfbard.mpq");
	diabdat_mpqs[MPQ_HF_BARB] = init_test_access("hfbarb.mpq");
	diabdat_mpqs[MPQ_HF_MUSIC] = init_test_access("hfmusic.mpq");
	diabdat_mpqs[MPQ_HF_VOICE] = init_test_access("hfvoice.mpq");
	diabdat_mpqs[MPQ_HF_OPT1] = init_test_access("hfopt1.mpq");
	diabdat_mpqs[MPQ_HF_OPT2] = init_test_access("hfopt2.mpq");
	diabdat_mpqs[MPQ_DEVILUTIONX] = init_test_access("devilutionx.mpq");
#endif
	diabdat_mpqs[MPQ_DEVILX] = init_test_access("devilx.mpq");
	if (diabdat_mpqs[MPQ_DEVILX] == NULL)
		app_fatal("Can not find/access '%s' in the game folder.", "devilx.mpq");

#ifdef MPQONE
	int i;
	// first round - read the content and prepare the metadata
	std::ifstream input("listfiles.txt");
	if (input.fail()) {
		app_fatal("To create a merged MPQ, place the 'listfiles.txt' in the game folder.");
	}
	std::string line;
	int entryCount = 0;
	while (std::getline(input, line)) {
		for (i = 0; i < NUM_MPQS; i++) {
			if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_CHECK_EXISTS, NULL)) {
				entryCount++;
				break;
			}
		}
	}
	input.close();

	// calculate the required number of hashes
	// TODO: use GetNearestPowerOfTwo of StormCommon.h?
	int hashCount = 1;
	while (hashCount <= entryCount) {
		hashCount <<= 1;
	}
	// create the mpq file
	input.open("listfiles.txt");
	std::string path = std::string(GetBasePath()) + MPQONE;
	if (!OpenMPQ(path.c_str(), hashCount, hashCount))
		app_fatal("Unable to open MPQ file %s.", path.c_str());
	while (std::getline(input, line)) {
		for (i = 0; i < NUM_MPQS; i++) {
			HANDLE hFile;
			if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_FROM_MPQ, &hFile)) {
				DWORD dwLen = SFileGetFileSize(hFile);
				BYTE *buf = DiabloAllocPtr(dwLen);
				if (!SFileReadFile(hFile, buf, dwLen, NULL))
					app_fatal("Unable to read save file");
				if (!mpqapi_write_file(line.c_str(), buf, dwLen))
					app_fatal("Unable to write %s to the MPQ.", line.c_str());
				mem_free_dbg(buf);
				SFileCloseFile(hFile);
				break;
			}
		}
	}
	input.close();
	mpqapi_flush_and_close(true);

	// cleanup
	for (i = 0; i < NUM_MPQS; i++) {
		if (diabdat_mpqs[i] != NULL) {
			SFileCloseArchive(diabdat_mpqs[i]);
		}
	}
	diabdat_mpq = init_test_access(MPQONE);
	assert(diabdat_mpq != NULL);
#endif
}

void init_create_window()
{
	if (!SpawnWindow(PROJECT_NAME))
		app_fatal("Unable to create main window");
	dx_init();
	gbActive = true;
	SDL_DisableScreenSaver();
}

void MainWndProc(UINT Msg, WPARAM wParam, LPARAM lParam)
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

WNDPROC SetWindowProc(WNDPROC newWndProc)
{
	WNDPROC oldWndProc;

	oldWndProc = CurrentWndProc;
	CurrentWndProc = newWndProc;
	return oldWndProc;
}

DEVILUTION_END_NAMESPACE
