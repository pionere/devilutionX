/**
 * @file init.cpp
 *
 * Implementation of routines for initializing the environment, disable screen saver, load MPQ.
 */
//#if defined(_WIN32)
//#include <find_steam_game.h>
//#endif

#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include "storm/storm_cfg.h"
#include <string>
#if CREATE_MPQONE
#include <fstream>
#endif

#ifdef __vita__
// increase default allowed heap size on Vita
int _newlib_heap_size_user = 100 * 1024 * 1024;
#endif

DEVILUTION_BEGIN_NAMESPACE

/** A handle to the mpq archives. */
#if USE_MPQONE
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
#if USE_MPQONE
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

static void ReadOnlyTest()
{
	std::string path = GetPrefPath();
	path += "Diablo1ReadOnlyTest.foo";
	FILE* f = FileOpen(path.c_str(), "w");
	if (f != NULL) {
		fclose(f);
		RemoveFile(path.c_str());
	} else {
		app_fatal("Unable to write to location:\n%s", GetPrefPath());
	}
}

void InitArchives()
{
	InitializeMpqCryptography();
	ReadOnlyTest();
	bool directFileAccess = getIniBool("Diablo", "Direct FileAccess", false);
	SFileEnableDirectAccess(directFileAccess);
#if USE_MPQONE
	diabdat_mpq = init_test_access(MPQONE);
	if (diabdat_mpq != NULL)
		return;
#if !CREATE_MPQONE
	//if (!directFileAccess)
	//	app_fatal("Can not find/access '%s' in the game folder.", MPQONE);
	return;
#endif
	HANDLE diabdat_mpqs[NUM_MPQS];
/*#elif defined(_WIN32)
	char gogpath[_FSG_PATH_MAX];
	fsg_get_gog_game_path(gogpath, "1412601690");
	if (strlen(gogpath) > 0) {
		paths.emplace_back(std::string(gogpath) + "/");
		paths.emplace_back(std::string(gogpath) + "/hellfire/");
	}*/
#else
	HANDLE diabdat_mpq = init_test_access(MPQONE);
	if (diabdat_mpq != NULL) {
		diabdat_mpqs[0] = diabdat_mpq;
		return;
	}
#endif
	diabdat_mpqs[MPQ_DIABDAT] = init_test_access(DATA_ARCHIVE_MAIN);
	if (diabdat_mpqs[MPQ_DIABDAT] == NULL)
		diabdat_mpqs[MPQ_DIABDAT] = init_test_access(DATA_ARCHIVE_MAIN_ALT);
	//if (!directFileAccess && diabdat_mpqs[MPQ_DIABDAT] == NULL)
	//	app_fatal("Can not find/access '%s' in the game folder.", DATA_ARCHIVE_MAIN);
	//diabdat_mpqs[MPQ_PATCH_RT] = init_test_access(DATA_ARCHIVE_PATCH);
	//if (SFileReadArchive(diabdat_mpqs[MPQ_DIABDAT], "ui_art\\title.pcx", NULL) == 0)
	//	InsertCDDlg();

#ifdef HELLFIRE
	diabdat_mpqs[MPQ_HELLFIRE] = init_test_access("hellfire.mpq");
	diabdat_mpqs[MPQ_HF_MONK] = init_test_access("hfmonk.mpq");
	//diabdat_mpqs[MPQ_HF_BARD] = init_test_access("hfbard.mpq");
	//diabdat_mpqs[MPQ_HF_BARB] = init_test_access("hfbarb.mpq");
	diabdat_mpqs[MPQ_HF_MUSIC] = init_test_access("hfmusic.mpq");
	diabdat_mpqs[MPQ_HF_VOICE] = init_test_access("hfvoice.mpq");
	//diabdat_mpqs[MPQ_HF_OPT1] = init_test_access("hfopt1.mpq");
	//diabdat_mpqs[MPQ_HF_OPT2] = init_test_access("hfopt2.mpq");
#endif
	diabdat_mpqs[MPQ_DEVILX] = init_test_access("devilx.mpq");
	//if (!directFileAccess && diabdat_mpqs[MPQ_DEVILX] == NULL)
	//	app_fatal("Can not find/access '%s' in the game folder.", "devilx.mpq");
#if ASSET_MPL != 1
	char tmpstr[32];
	snprintf(tmpstr, lengthof(tmpstr), "devilx_hd%d.mpq", ASSET_MPL);
	diabdat_mpqs[MPQ_DEVILHD] = init_test_access(tmpstr);
	//if (!directFileAccess && diabdat_mpqs[MPQ_DEVILHD] == NULL)
	//	app_fatal("Can not find/access '%s' in the game folder.", tmpstr);
#endif

#if CREATE_MPQONE
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
			if (diabdat_mpqs[i] == NULL) continue;
			if (SFileReadArchive(diabdat_mpqs[i], line.c_str(), NULL) != 0) {
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
	HANDLE ha = SFileCreateArchive(path.c_str(), hashCount, hashCount);
	if (ha == NULL)
		app_fatal("Unable to create MPQ file %s.", path.c_str());
	while (std::getline(input, line)) {
#ifdef NOSOUND
		if (line.size() >= 4 && SDL_strcasecmp(line.c_str() + line.size() - 4, ".wav") == 0)
			continue;
#endif
		for (i = 0; i < NUM_MPQS; i++) {
			if (diabdat_mpqs[i] == NULL) continue;
			BYTE* buf = NULL;
			DWORD dwLen = SFileReadArchive(diabdat_mpqs[i], line.c_str(), &buf);
			if (dwLen != 0) {
				bool success = SFileWriteFile(ha, line.c_str(), buf, dwLen);
				mem_free_dbg(buf);
				if (!success) {
					SFileCloseArchive(ha);
					app_fatal("Unable to write %s to the MPQ.", line.c_str());
				}
				break;
			}
		}
	}
	input.close();
	SFileFlushAndCloseArchive(ha);

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
