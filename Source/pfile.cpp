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

DEVILUTION_BEGIN_NAMESPACE

#define SAVEFILE_GAME             "game"
#define SAVEFILE_HERO             "hero"
#define PFILE_SAVE_MPQ_HASHCOUNT  2048
#define PFILE_SAVE_MPQ_BLOCKCOUNT 2048
#define PFILE_SAVE_INTERVAL       60000

unsigned mySaveIdx;
bool gbValidSaveFile;
static Uint32 guNextSaveTc;

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

static bool pfile_read_hero(HANDLE archive, PkPlayerStruct* pPack)
{
	HANDLE file;
	DWORD dwlen;
	BYTE* buf;

	if (!SFileOpenFileEx(archive, SAVEFILE_HERO, SFILE_OPEN_FROM_MPQ, &file)) {
		return false;
	} else {
		bool ret = false;
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		dwlen = SFileGetFileSize(file);
		if (dwlen != 0) {
			buf = DiabloAllocPtr(dwlen);
			if (SFileReadFile(file, buf, dwlen)) {
				int read = codec_decode(buf, dwlen, password);
				if (read == sizeof(*pPack)) {
					memcpy(pPack, buf, sizeof(*pPack));
					ret = true;
				}
			}
			mem_free_dbg(buf);
		}
		SFileCloseFile(file);
		return ret;
	}
}

static void pfile_encode_hero(int pnum)
{
	BYTE* packed;
	PkPlayerStruct pPack;
	DWORD packed_len;
	const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

	PackPlayer(&pPack, pnum);
	packed_len = codec_get_encoded_len(sizeof(pPack));
	packed = (BYTE*)DiabloAllocPtr(packed_len);
	memcpy(packed, &pPack, sizeof(pPack));
	codec_encode(packed, sizeof(pPack), packed_len, password);
	mpqapi_write_entry(SAVEFILE_HERO, packed, packed_len);
	mem_free_dbg(packed);
}

static bool pfile_open_save_mpq(unsigned save_num)
{
	return OpenMPQ(GetSavePath(save_num).c_str(), PFILE_SAVE_MPQ_HASHCOUNT, PFILE_SAVE_MPQ_BLOCKCOUNT);
}

static bool pfile_open_archive()
{
	return pfile_open_save_mpq(mySaveIdx);
}

void pfile_flush(bool bFree)
{
	mpqapi_flush_and_close(bFree);
}

static HANDLE pfile_open_save_archive(unsigned save_num)
{
	return SFileOpenArchive(GetSavePath(save_num).c_str(), MPQ_OPEN_READ_ONLY);
}

void pfile_write_hero(bool bFree)
{
	if (pfile_open_archive()) {
		pfile_encode_hero(mypnum);
		pfile_flush(bFree);
	}
}

static void pfile_player2hero(const PlayerStruct* p, _uiheroinfo* heroinfo, unsigned saveIdx, bool bHasSaveFile)
{
	memset(heroinfo->hiName, 0, sizeof(heroinfo->hiName));
	SStrCopy(heroinfo->hiName, p->_pName, sizeof(heroinfo->hiName));
	heroinfo->hiIdx = saveIdx;
	heroinfo->hiLevel = p->_pLevel;
	heroinfo->hiClass = p->_pClass;
	heroinfo->hiRank = p->_pRank;
	heroinfo->hiStrength = p->_pStrength;
	heroinfo->hiMagic = p->_pMagic;
	heroinfo->hiDexterity = p->_pDexterity;
	heroinfo->hiVitality = p->_pVitality;
	heroinfo->hiHasSaved = bHasSaveFile;
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

/*bool pfile_rename_hero(const char* name_1, const char* name_2)
{
	int i;
	unsigned save_num;
	_uiheroinfo uihero;
	bool found = false;

	if (!ValidPlayerName(name_2))
		return false;

	SStrCopy(players[i]._pName, name_2, PLR_NAME_LEN);
	pfile_player2hero(&players[0], &uihero, mySaveIdx, gbValidSaveFile);
	pfile_write_hero();
	return true;
}*/

static bool pfile_archive_contains_game(HANDLE hsArchive)
{
	if (IsMultiGame)
		return false;

	return SFileOpenFileEx(hsArchive, SAVEFILE_GAME, SFILE_OPEN_CHECK_EXISTS, NULL);
}

void pfile_ui_load_hero_infos(std::vector<_uiheroinfo> &hero_infos)
{
	int i;

	for (i = 0; i < MAX_CHARACTERS; i++) {
		PkPlayerStruct pkplr;
		HANDLE archive = pfile_open_save_archive(i);
		if (archive != NULL) {
			if (pfile_read_hero(archive, &pkplr)) {
				UnPackPlayer(&pkplr, 0);
				_uiheroinfo uihero;
				pfile_player2hero(&players[0], &uihero, i, pfile_archive_contains_game(archive));
				hero_infos.push_back(uihero);
			}
			SFileCloseArchive(archive);
		}
	}
}

/*void pfile_ui_set_class_stats(unsigned int player_class_nr, _uidefaultstats* class_stats)
{
	class_stats->dsStrength = StrengthTbl[player_class_nr];
	class_stats->dsMagic = MagicTbl[player_class_nr];
	class_stats->dsDexterity = DexterityTbl[player_class_nr];
	class_stats->dsVitality = VitalityTbl[player_class_nr];
}*/

int pfile_ui_create_save(_uiheroinfo* heroinfo)
{
	unsigned save_num;
	HANDLE archive;

	if (!ValidPlayerName(heroinfo->hiName))
		return NEWHERO_INVALID_NAME;
	assert(heroinfo->hiIdx == MAX_CHARACTERS);
	for (save_num = 0; save_num < MAX_CHARACTERS; save_num++) {
		archive = pfile_open_save_archive(save_num);
		if (archive == NULL)
			break;
		SFileCloseArchive(archive);
	}
	if (save_num >= MAX_CHARACTERS)
		return NEWHERO_HERO_LIMIT;
	if (!pfile_open_save_mpq(save_num))
		return NEWHERO_FAIL;
	static_assert(MAX_CHARACTERS <= UCHAR_MAX, "Save-file index does not fit to _uiheroinfo.");
	heroinfo->hiIdx = save_num;
	//mpqapi_remove_entries(pfile_get_file_name);
	CreatePlayer(*heroinfo);
	pfile_encode_hero(0);
	//pfile_player2hero(&players[0], heroinfo, save_num, false);
	pfile_flush(true);
	return NEWHERO_DONE;
}

static bool GetPermLevelNames(unsigned dwIndex, char (&szPerm)[DATA_ARCHIVE_MAX_PATH])
{
	const char* fmt;

	static_assert(NUM_LEVELS < 100, "PermSaveNames are too short to fit the number of levels.");
	if (dwIndex < NUM_STDLVLS)
		fmt = "perml%02d";
	else if (dwIndex < NUM_LEVELS) {
		dwIndex -= NUM_STDLVLS;
		fmt = "perms%02d";
	} else
		return false;

	snprintf(szPerm, sizeof(szPerm), fmt, dwIndex);
	return true;
}

static bool GetTempLevelNames(unsigned dwIndex, char (&szTemp)[DATA_ARCHIVE_MAX_PATH])
{
	const char* fmt;

	static_assert(NUM_LEVELS < 100, "TempSaveNames are too short to fit the number of levels.");
	if (dwIndex < NUM_STDLVLS)
		fmt = "templ%02d";
	else if (dwIndex < NUM_LEVELS) {
		dwIndex -= NUM_STDLVLS;
		fmt = "temps%02d";
	} else
		return false;

	snprintf(szTemp, sizeof(szTemp), fmt, dwIndex);
	return true;
}

/*bool pfile_get_file_name(unsigned lvl, char (&dst)[DATA_ARCHIVE_MAX_PATH])
{
	if (IsMultiGame) {
		if (lvl != 0)
			return false;
		copy_cstr(dst, SAVEFILE_HERO);
	} else {
		if (lvl < NUM_LEVELS)
			return GetPermLevelNames(lvl, dst);
		if (lvl == NUM_LEVELS)
			copy_cstr(dst, SAVEFILE_GAME);
		else if (lvl == NUM_LEVELS + 1)
			copy_cstr(dst, SAVEFILE_HERO);
		else
			return false;
	}
	return true;
}*/

void pfile_ui_delete_save(_uiheroinfo* hero_info)
{
	unsigned save_num;

	save_num = hero_info->hiIdx;
	assert(save_num < MAX_CHARACTERS);
	RemoveFile(GetSavePath(save_num).c_str());
}

void pfile_read_hero_from_save()
{
	HANDLE archive;
	PkPlayerStruct pkplr;

	archive = pfile_open_save_archive(mySaveIdx);
	if (archive == NULL)
		app_fatal("Unable to open file archive");
	if (!pfile_read_hero(archive, &pkplr))
		app_fatal("Unable to read save file");

	UnPackPlayer(&pkplr, 0); // mypnum
	mypnum = 0;
	gbValidSaveFile = pfile_archive_contains_game(archive);
	SFileCloseArchive(archive);
	guNextSaveTc = SDL_GetTicks() + PFILE_SAVE_INTERVAL;
}

void pfile_rename_temp_to_perm()
{
	unsigned dwIndex;
	bool bResult;
	char szTemp[DATA_ARCHIVE_MAX_PATH];
	char szPerm[DATA_ARCHIVE_MAX_PATH];

	assert(!IsMultiGame);
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");

	dwIndex = 0;
	while (GetTempLevelNames(dwIndex, szTemp)) {
		bResult = GetPermLevelNames(dwIndex, szPerm);
		assert(bResult);
		dwIndex++;
		if (mpqapi_has_entry(szTemp)) {
			if (mpqapi_has_entry(szPerm))
				mpqapi_remove_entry(szPerm);
			mpqapi_rename_entry(szTemp, szPerm);
		}
	}
	assert(!GetPermLevelNames(dwIndex, szPerm));
	pfile_flush(true);
}

void pfile_write_save_file(bool full, DWORD dwLen)
{
	DWORD qwLen;
	char pszName[DATA_ARCHIVE_MAX_PATH] = SAVEFILE_GAME;
	BYTE* pbData = gsDeltaData.ddBuffer;

	qwLen = codec_get_encoded_len(dwLen);

	{
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		codec_encode(pbData, dwLen, qwLen, password);
	}
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");

	if (!full)
		GetTempLevelNames(currLvl._dLevelIdx, pszName);
	mpqapi_write_entry(pszName, pbData, qwLen);
	pfile_flush(true);
}

void pfile_delete_save_file(bool full)
{
	if (!IsMultiGame) {
		if (!pfile_open_archive())
			app_fatal("Unable to open file archive");
		if (full)
			mpqapi_remove_entry(SAVEFILE_GAME);
		else
			mpqapi_remove_entries(GetTempLevelNames);
		pfile_flush(true);
	}
}

void pfile_read_save_file(bool full)
{
	DWORD len;
	HANDLE archive, save;
	char pszName[DATA_ARCHIVE_MAX_PATH] = SAVEFILE_GAME;
	int source;

	archive = pfile_open_save_archive(mySaveIdx);
	if (archive == NULL)
		app_fatal("Unable to open file archive");

	source = 0;
nextSource:
	if (!full) {
		if (source == 0)
			GetTempLevelNames(currLvl._dLevelIdx, pszName);
		else
			GetPermLevelNames(currLvl._dLevelIdx, pszName);
	}

	if (!SFileOpenFileEx(archive, pszName, SFILE_OPEN_FROM_MPQ, &save)) {
		source++;
		if (source == 1) {
			goto nextSource;
		}
		app_fatal("Unable to open save file");
	}

	len = SFileGetFileSize(save);
	if (len == 0 || len > sizeof(gsDeltaData.ddBuffer))
		app_fatal("Invalid save file");

	if (!SFileReadFile(save, gsDeltaData.ddBuffer, len))
		app_fatal("Unable to read save file");
	SFileCloseFile(save);
	SFileCloseArchive(archive);

	{
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		len = codec_decode(gsDeltaData.ddBuffer, len, password);
		if (len == 0) {
			app_fatal("Invalid save file");
		}
	}
}

void pfile_update(bool force_save)
{
	if (IsMultiGame) {
		Uint32 currTc = SDL_GetTicks();
		if (force_save || currTc > guNextSaveTc) {
			guNextSaveTc = currTc + PFILE_SAVE_INTERVAL;
			pfile_write_hero(false);
		}
	}
}

DEVILUTION_END_NAMESPACE
