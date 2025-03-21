/**
 * @file pfile.cpp
 *
 * Implementation of the save game encoding functionality.
 */
#include <string>

#include "all.h"
#include "engine/render/text_render.h"
#include "utils/paths.h"
#include "diabloui.h"
#include "utils/file_util.h"
#include "DiabloUI/diablo.h"
#include <ctime>

DEVILUTION_BEGIN_NAMESPACE

#define SAVEFILE_GAME             "game"
#define SAVEFILE_HERO             "hero"
#define PFILE_ENTRY_MAX_PATH      8
#define PFILE_MPQ_HASHCOUNT_SINGLE  2048
#define PFILE_MPQ_BLOCKCOUNT_SINGLE 128
#define PFILE_MPQ_HASHCOUNT_MULTI   1
#define PFILE_MPQ_BLOCKCOUNT_MULTI  1
#define PFILE_SAVE_INTERVAL       2048

static_assert(DATA_ARCHIVE_MAX_PATH >= PFILE_ENTRY_MAX_PATH, "pfile can not write to the mpq archive.");
static_assert((PFILE_MPQ_HASHCOUNT_SINGLE & (PFILE_MPQ_HASHCOUNT_SINGLE - 1)) == 0, "hash count must be a power of 2 (single).");
static_assert(PFILE_MPQ_HASHCOUNT_SINGLE >= PFILE_MPQ_BLOCKCOUNT_SINGLE, "not enough hash entry to store the blocks of a single-player game");
static_assert(PFILE_MPQ_BLOCKCOUNT_SINGLE >= 2 * (2 * NUM_FIXLVLS + 1 + 1) + 1, "block table is too small to store every entry of a single-player game."); // 2 * entries_with_dynamic_size + entries_with_fix_size
static_assert((PFILE_MPQ_HASHCOUNT_MULTI & (PFILE_MPQ_HASHCOUNT_MULTI - 1)) == 0, "hash count must be a power of 2 (multi).");
static_assert(PFILE_MPQ_HASHCOUNT_MULTI >= PFILE_MPQ_BLOCKCOUNT_MULTI, "not enough hash entry to store the blocks of a multi-player game");
static_assert(PFILE_MPQ_BLOCKCOUNT_MULTI >= 1, "block table is too small to store every entry of a multi-player game.");

/* the selected hero of the local player */
unsigned mySaveIdx;
/* the handle of the archive containing the hero of the multiplayer game */
static HANDLE archive;
/* the last turn when the hero of the multiplayer game was saved. */
uint32_t guLastSaveTurn;

#define PASSWORD_SINGLE "xrgyrkj1"
#define PASSWORD_MULTI  "szqnlsk1"

#ifdef HELLFIRE
#define SAVE_FILE_FORMAT_SINGLE "single_%d.hsv"
#define SAVE_FILE_FORMAT_MULTI  "multi_%d.hsv"
#else
#define SAVE_FILE_FORMAT_SINGLE "single_%d.sv"
#define SAVE_FILE_FORMAT_MULTI  "multi_%d.sv"
#endif

static std::string GetSavePath(unsigned save_num)
{
	std::string path = GetPrefPath();

	const char* fmt = IsMultiGame ? SAVE_FILE_FORMAT_MULTI : SAVE_FILE_FORMAT_SINGLE;
	static_assert(MAX_CHARACTERS < 100, "Name of the save-file does not fit to the temporary buffer.");
	char save_file_name[std::max(sizeof(SAVE_FILE_FORMAT_MULTI), sizeof(SAVE_FILE_FORMAT_SINGLE))];
	snprintf(save_file_name, sizeof(save_file_name), fmt, save_num);
	path.append(save_file_name);
	return path;
}

static bool pfile_archive_read_hero(HANDLE ha, PkPlayerStruct* pPack)
{
	bool ret = false;
	BYTE* buf = NULL;
	DWORD dwLen = SFileReadArchive(ha, SAVEFILE_HERO, &buf);
	if (dwLen != 0) {
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;
		int read = codec_decode(buf, dwLen, password);
		if (read == sizeof(*pPack)) {
			memcpy(pPack, buf, sizeof(*pPack));
			ret = true;
		}
		mem_free_dbg(buf);
	}
	return ret;
}

static bool pfile_archive_encode_hero(HANDLE ha, int pnum)
{
	const DWORD packed_len = codec_get_encoded_len(sizeof(PkPlayerStruct));
	BYTE* packed;

	packed = (BYTE*)DiabloAllocPtr(packed_len);
	PackPlayer((PkPlayerStruct*)packed, pnum);
	{
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		codec_encode(packed, sizeof(PkPlayerStruct), packed_len, password);
	}
	const bool success = SFileWriteFile(ha, SAVEFILE_HERO, packed, packed_len);
	mem_free_dbg(packed);
	return success;
}

static HANDLE pfile_archive_open_save(unsigned save_num, DWORD dwFlags)
{
	return SFileOpenArchive(GetSavePath(save_num).c_str(), dwFlags);
}

static void pfile_archive_write_hero()
{
	if (SFileReopenArchive(archive, GetSavePath(mySaveIdx).c_str())) {
		pfile_archive_encode_hero(archive, mypnum);
		SFileFlushArchive(archive);
	}
}

static void pfile_player2hero(const PlayerStruct* p, _uiheroinfo* heroinfo)
{
	static_assert(sizeof(heroinfo->hiName) <= sizeof(p->_pName), "pfile_player2hero uses memcpy to store the name of the player.");
	memcpy(heroinfo->hiName, p->_pName, sizeof(heroinfo->hiName));
	heroinfo->hiName[sizeof(heroinfo->hiName) - 1] = '\0';
	// heroinfo->hiIdx;
	heroinfo->hiLevel = p->_pLevel;
	heroinfo->hiClass = p->_pClass;
	// heroinfo->hiSaveFile
	heroinfo->hiStrength = p->_pStrength;
	heroinfo->hiMagic = p->_pMagic;
	heroinfo->hiDexterity = p->_pDexterity;
	heroinfo->hiVitality = p->_pVitality;
}

static bool ValidPlayerName(const char* name)
{
	//int i;
	//const BYTE invalidChars[] = ",<>%&\\\"?*#/: ";

	if (name[0] == '\0')
		return false;

	for (BYTE* letter = (BYTE*)name; *letter != '\0'; letter++) {
		//for (i = 0; i < lengthof(invalidChars); i++) {
		//	if (*letter == invalidChars[i])
		//		return false;
		//}
		if (*letter != 0x20 && gbStdFontFrame[*letter] == 0)
			return false;
	}

	return true;
}

/*bool pfile_rename_hero(_uiheroinfo* heroinfo, const char* name_2)
{
	bool result = false;

	if (ValidPlayerName(name_2)) {
		HANDLE ha = pfile_archive_open_save(heroinfo->hiIdx, MPQ_OPEN_READ_ONLY);
		if (ha != NULL) {
			PkPlayerStruct pkplr;
			if (pfile_archive_read_hero(ha, &pkplr)) {
				SStrCopy(pkplr.pName, name_2, lengthof(pkplr.pName));
				UnPackPlayer(&pkplr, 0);
				pfile_archive_encode_hero(ha, 0);
				SFileFlushAndCloseArchive(ha);
				result = true;
			}
		}
	}
	return result;
}*/

static bool pfile_archive_contains_game(HANDLE hsArchive)
{
	return SFileReadArchive(hsArchive, SAVEFILE_GAME, NULL) != 0;
}

void pfile_ui_load_heros(std::vector<_uiheroinfo> &hero_infos)
{
	int i;

	for (i = MAX_CHARACTERS; i >= 0; i--) {
		HANDLE ha = pfile_archive_open_save(i, MPQ_OPEN_READ_ONLY);
		if (ha != NULL) {
			PkPlayerStruct pkplr;
			if (pfile_archive_read_hero(ha, &pkplr)) {
				UnPackPlayer(&pkplr, 0);
				_uiheroinfo uihero;
				uihero.hiIdx = i;
				uihero.hiSaveFile = pfile_archive_contains_game(ha);
				pfile_player2hero(&players[0], &uihero);
				hero_infos.push_back(uihero);
			}
			SFileCloseArchive(ha);
		}
	}
}

int pfile_ui_create_hero(_uiheroinfo* heroinfo)
{
	unsigned save_num;
	std::string path;

	if (!ValidPlayerName(heroinfo->hiName))
		return NEWHERO_INVALID_NAME;
	assert(heroinfo->hiIdx == MAX_CHARACTERS + 1);
	for (save_num = 0; save_num <= MAX_CHARACTERS; save_num++) {
		path = GetSavePath(save_num);
		if (FileExists(path.c_str())) continue;
		const int hashCount = IsMultiGame ? PFILE_MPQ_HASHCOUNT_MULTI : PFILE_MPQ_HASHCOUNT_SINGLE;
		const int blockCount = IsMultiGame ? PFILE_MPQ_BLOCKCOUNT_MULTI : PFILE_MPQ_BLOCKCOUNT_SINGLE;
		HANDLE ha = SFileCreateArchive(path.c_str(), hashCount, blockCount);
		if (ha != NULL) {
			static_assert(MAX_CHARACTERS <= UCHAR_MAX, "Save-file index does not fit to _uiheroinfo.");
			heroinfo->hiIdx = save_num;
			// heroinfo->hiSaveFile = FALSE;
			CreatePlayer(*heroinfo);
			pfile_archive_encode_hero(ha, 0);
			//pfile_player2hero(&players[0], heroinfo);
			SFileFlushAndCloseArchive(ha);
			return NEWHERO_DONE;
		}
		return NEWHERO_FAIL;
	}
	return NEWHERO_HERO_LIMIT;
}

static void GetPermLevelNames(unsigned dwIndex, char (&szPerm)[PFILE_ENTRY_MAX_PATH])
{
	const char* fmt = "plvl%02d";

	static_assert(NUM_LEVELS < 100, "PermSaveNames are too short to fit the number of levels.");
	static_assert(PFILE_ENTRY_MAX_PATH >= sizeof("plvl**"), "PermSaveName can not be written to the path");

	snprintf(szPerm, sizeof(szPerm), fmt, dwIndex);
}

static void GetTempLevelNames(unsigned dwIndex, char (&szTemp)[PFILE_ENTRY_MAX_PATH])
{
	const char* fmt = "tlvl%02d";

	static_assert(NUM_LEVELS < 100, "TempSaveNames are too short to fit the number of levels.");
	static_assert(PFILE_ENTRY_MAX_PATH >= sizeof("tlvl**"), "TempSaveName can not be written to the path");

	snprintf(szTemp, sizeof(szTemp), fmt, dwIndex);
}

void pfile_ui_delete_hero(_uiheroinfo* hero_info)
{
	unsigned save_num;

	save_num = hero_info->hiIdx;
	assert(save_num <= MAX_CHARACTERS);
	RemoveFile(GetSavePath(save_num).c_str());
}

void pfile_read_hero()
{
	HANDLE ha;
	// const char* err = "Unable to open file archive";
	bool success = false;
	ha = pfile_archive_open_save(mySaveIdx, MPQ_OPEN_READ_ONLY);
	if (ha != NULL) {
		PkPlayerStruct pkplr;
		// err = "Unable to read save file";
		if (pfile_archive_read_hero(ha, &pkplr)) {
			UnPackPlayer(&pkplr, mypnum);
			// err = NULL;
			success = true;
			if (IsMultiGame) {
				SFileReleaseArchive(ha);
				archive = ha;
				ha = NULL;
			}
		}
		SFileCloseArchive(ha);
	}
	if (!success)
		app_fatal("Unable to read save file");
}

void pfile_write_save_file(bool full, DWORD dwLen)
{
	DWORD qwLen;
	char pszName[PFILE_ENTRY_MAX_PATH] = SAVEFILE_GAME;
	BYTE* pbData = gsDeltaData.ddBuffer;

	qwLen = codec_get_encoded_len(dwLen);

	{
		// assert(!IsMultiGame);
		const char* password = /*IsMultiGame ? PASSWORD_MULTI :*/ PASSWORD_SINGLE;

		codec_encode(pbData, dwLen, qwLen, password);
	}
	HANDLE ha;
	ha = pfile_archive_open_save(mySaveIdx, 0);
	if (ha == NULL)
		app_fatal("Unable to open file archive");
	if (!full)
		GetTempLevelNames(currLvl._dLevelIdx, pszName);
	SFileWriteFile(ha, pszName, pbData, qwLen);
	if (full) {
		// gbValidSaveFile = true;
		// rename temp to perm
		char szTemp[PFILE_ENTRY_MAX_PATH];
		char szPerm[PFILE_ENTRY_MAX_PATH];

		for (int i = 0; i < NUM_LEVELS; i++) {
			GetTempLevelNames(i, szTemp);
			GetPermLevelNames(i, szPerm);
			SFileRenameFile(ha, szTemp, szPerm);
		}
		// assert(mypnum == 0);
		pfile_archive_encode_hero(ha, 0);
	}
	SFileFlushAndCloseArchive(ha);
}

void pfile_read_save_file(bool full)
{
	DWORD len;
	HANDLE ha;
	// const char* err = "Unable to open file archive";
	bool success = false;
	ha = pfile_archive_open_save(mySaveIdx, 0); // full ? 0 : MPQ_OPEN_READ_ONLY
	if (ha != NULL) {
		bool change = false;
		char pszName[PFILE_ENTRY_MAX_PATH] = SAVEFILE_GAME;
		BYTE* buf = NULL;
		if (full) {
			// delete the temp entries
			char szTemp[PFILE_ENTRY_MAX_PATH];
			for (int i = 0; i < NUM_LEVELS; i++) {
				GetTempLevelNames(i, szTemp);
				change |= SFileRemoveFile(ha, szTemp);
			}
		}
		if (!full) {
			GetTempLevelNames(currLvl._dLevelIdx, pszName);
		}
		len = SFileReadArchive(ha, pszName, &buf);
		if (len == 0 && !full) {
			GetPermLevelNames(currLvl._dLevelIdx, pszName);
			len = SFileReadArchive(ha, pszName, &buf);
		}
		if (len != 0) {
			// assert(!IsMultiGame);
			const char* password = /*IsMultiGame ? PASSWORD_MULTI :*/ PASSWORD_SINGLE;

			len = codec_decode(buf, len, password);
			if (len != 0 && len <= sizeof(gsDeltaData.ddBuffer)) {
				// err = NULL;
				memcpy(gsDeltaData.ddBuffer, buf, len);
				success = true;
			}
			mem_free_dbg(buf);
		}
		if (change)
			SFileFlushAndCloseArchive(ha);
		else
			SFileCloseArchive(ha);
	}
	//if (err != NULL)
	//	app_fatal(err);
	if (!success)
		app_fatal("Unable to read save file");
}

void pfile_update(bool force_save)
{
	if (archive != NULL) {
		// assert(IsMultiGame);
		uint32_t currTurn = gdwGameLogicTurn;
		if (force_save || ((currTurn - guLastSaveTurn) % PFILE_SAVE_INTERVAL) == 0) {
			guLastSaveTurn = currTurn;
			pfile_archive_write_hero();
		}
	}
}

void pfile_close()
{
	if (archive != NULL) {
		// assert(IsMultiGame);
		pfile_archive_write_hero();
		SFileCloseArchive(archive);
		archive = NULL;
	}
}

DEVILUTION_END_NAMESPACE
