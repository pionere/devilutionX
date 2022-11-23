/**
 * @file init.cpp
 *
 * Implementation of routines for initializing the environment, disable screen saver, load MPQ.
 */
//#if defined(_WIN64) || defined(_WIN32)
//#include <find_steam_game.h>
//#endif

#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include <SDL.h>
#include <string>
#include <fstream>
#if DEV_MODE
#include <sys/stat.h>
#endif

#ifdef __vita__
// increase default allowed heap size on Vita
int _newlib_heap_size_user = 100 * 1024 * 1024;
#endif

DEVILUTION_BEGIN_NAMESPACE

/** A handle to the mpq archives. */
#ifdef MPQONE
HANDLE diabdat_mpq;
#else
HANDLE diabdat_mpqs[NUM_MPQS];
#endif

static HANDLE init_test_access(const char* mpq_name)
{
	HANDLE archive;
#if defined(__3DS__) || defined(__SWITCH__)
	const char* paths[3] = { GetBasePath(), GetPrefPath(), "romfs:/" };
#elif defined(__unix__) && !defined(__ANDROID__)
	const char* paths[4] = { GetBasePath(), GetPrefPath(),
		"/usr/share/diasurgical/devilutionx/",
		"/usr/local/share/diasurgical/devilutionx/" };
#else
	const char* paths[2] = { GetBasePath(), GetPrefPath() };
#endif
	std::string mpq_abspath;
	for (int i = 0; i < lengthof(paths); i++) {
		mpq_abspath = paths[i];
		mpq_abspath += mpq_name;
		archive = SFileOpenArchive(mpq_abspath.c_str(), MPQ_OPEN_READ_ONLY);
		if (archive != NULL) {
			return archive;
		}
	}

	return NULL;
}

/* data */

void FreeArchives()
{
	pfile_flush(true);

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

#if DEV_MODE
static void CreateMpq(const char* destMpqName, const char* folder, const char* files)
{
	if (FileExists(destMpqName))
		return;

	std::string basePath = std::string(GetBasePath()) + folder;
	std::ifstream input(std::string(GetBasePath()) + files);

	int entryCount = 0;
	std::string line;
	while (std::getline(input, line)) {
		if (line[0] == '_')
			continue;
		std::string path = basePath + line.c_str();
		FILE* fp = fopen(path.c_str(), "r");
		if (fp == NULL)
			app_fatal("Missing file: %s", path.c_str());
		fclose(fp);
		entryCount++;
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
		FILE* fp = fopen(path.c_str(), "rb");
		if (fp != NULL) {
			struct stat st;
			stat(path.c_str(), &st);
			BYTE* buf = DiabloAllocPtr(st.st_size);
			int readBytes = fread(buf, 1, st.st_size, fp);
			fclose(fp);
			if (!mpqapi_write_entry(line.c_str(), buf, st.st_size))
				app_fatal("Unable to write %s to the MPQ.", line.c_str());
			mem_free_dbg(buf);
		}
	}
	input.close();
	mpqapi_flush_and_close(true);
}
#endif

static void ReadOnlyTest()
{
	std::string path = GetPrefPath();
	path += "Diablo1ReadOnlyTest.foo";
	FILE* f = FileOpen(path.c_str(), "w");
	if (f != NULL) {
		fclose(f);
		remove(path.c_str());
	} else {
		app_fatal("Unable to write to location:\n%s", GetPrefPath());
	}
}

void InitArchives()
{
	InitializeMpqCryptography();
	ReadOnlyTest();
	SFileEnableDirectAccess(getIniBool("Diablo", "Direct FileAccess", false));

	//CreateMpq("devilx.mpq", "Work\\", "mpqfiles.txt");
	//CreateMpq("devilx_hd2.mpq", "WorkHd\\", "hdfiles.txt");
#ifdef MPQONE
	diabdat_mpq = init_test_access(MPQONE);
	if (diabdat_mpq != NULL)
		return;

	HANDLE diabdat_mpqs[NUM_MPQS];
/*#elif defined(_WIN64) || defined(_WIN32)
	char gogpath[_FSG_PATH_MAX];
	fsg_get_gog_game_path(gogpath, "1412601690");
	if (strlen(gogpath) > 0) {
		paths.emplace_back(std::string(gogpath) + "/");
		paths.emplace_back(std::string(gogpath) + "/hellfire/");
	}*/
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
#endif
	diabdat_mpqs[MPQ_DEVILX] = init_test_access("devilx.mpq");
	if (diabdat_mpqs[MPQ_DEVILX] == NULL)
		app_fatal("Can not find/access '%s' in the game folder.", "devilx.mpq");
#if ASSET_MPL != 1
	char tmpstr[32];
	snprintf(tmpstr, lengthof(tmpstr), "devilx_hd%d.mpq", ASSET_MPL);
	diabdat_mpqs[MPQ_DEVILHD] = init_test_access(tmpstr);
	if (diabdat_mpqs[MPQ_DEVILHD] == NULL)
		app_fatal("Can not find/access '%s' in the game folder.", tmpstr);
#endif

#ifdef MPQONE
	int i;
	// first round - read the content and prepare the metadata
	std::string listpath = std::string(GetBasePath()) + "listfiles.txt";
	std::ifstream input(listpath);
	if (input.fail()) {
		app_fatal("Can not find/access '%s' in the game folder.", "listfiles.txt");
	}
	std::string line;
	int entryCount = 0;
	while (std::getline(input, line)) {
		for (i = 0; i < NUM_MPQS; i++) {
			//if (diabdat_mpqs[i] != NULL && SFileHasFile(diabdat_mpqs[i], line.c_str())) {
			if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_CHECK_EXISTS, NULL)) {
				entryCount++;
				break;
			}
		}
	}

	// calculate the required number of hashes
	// TODO: use GetNearestPowerOfTwo of StormCommon.h?
	int hashCount = 1;
	while (hashCount <= entryCount) {
		hashCount <<= 1;
	}
	// create the mpq file
	input.clear();                 // clear fail and eof bits
	input.seekg(0, std::ios::beg); // back to the start!
	std::string path = std::string(GetBasePath()) + MPQONE;
	if (!OpenMPQ(path.c_str(), hashCount, hashCount))
		app_fatal("Unable to open MPQ file %s.", path.c_str());
	while (std::getline(input, line)) {
#ifdef NOSOUND
		if (line.size() >= 4 && SDL_strcasecmp(line.c_str() + line.size() - 4, ".wav") == 0)
			continue;
#endif
		for (i = 0; i < NUM_MPQS; i++) {
			HANDLE hFile;
			if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_FROM_MPQ, &hFile)) {
				DWORD dwLen = SFileGetFileSize(hFile);
				BYTE* buf = DiabloAllocPtr(dwLen);
				if (!SFileReadFile(hFile, buf, dwLen))
					app_fatal("Unable to open file archive");
				if (!mpqapi_write_entry(line.c_str(), buf, dwLen))
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

DEVILUTION_END_NAMESPACE
