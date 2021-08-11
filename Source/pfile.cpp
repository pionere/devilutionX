/**
 * @file pfile.cpp
 *
 * Implementation of the save game encoding functionality.
 */
#include <string>

#include "all.h"
#include "utils/paths.h"
#include "diabloui.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

#define PFILE_SAVE_MPQ_HASHCOUNT	2048
#define PFILE_SAVE_MPQ_BLOCKCOUNT	2048
#define PFILE_SAVE_INTERVAL			60000

unsigned mySaveIdx;
bool gbValidSaveFile;
static Uint32 guNextSaveTc;

static const char *PASSWORD_SINGLE = "xrgyrkj1";
static const char *PASSWORD_MULTI = "szqnlsk1";

#ifdef HELLFIRE
#define SAVE_FILE_FORMAT_SINGLE "single_%d.hsv"
#define SAVE_FILE_FORMAT_MULTI "multi_%d.hsv"
#else
#define SAVE_FILE_FORMAT_SINGLE "single_%d.sv"
#define SAVE_FILE_FORMAT_MULTI "multi_%d.sv"
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

static bool pfile_read_hero(HANDLE archive, PkPlayerStruct *pPack)
{
	HANDLE file;
	DWORD dwlen;
	BYTE *buf;

	if (!SFileOpenFileEx(archive, SAVEFILE_HERO, SFILE_OPEN_FROM_MPQ, &file)) {
		return false;
	} else {
		bool ret = false;
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		dwlen = SFileGetFileSize(file);
		if (dwlen != 0) {
			DWORD read;
			buf = DiabloAllocPtr(dwlen);
			if (SFileReadFile(file, buf, dwlen, &read)) {
				read = codec_decode(buf, dwlen, password);
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

static void pfile_encode_hero(const PkPlayerStruct *pPack)
{
	BYTE *packed;
	DWORD packed_len;
	const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

	packed_len = codec_get_encoded_len(sizeof(*pPack));
	packed = (BYTE *)DiabloAllocPtr(packed_len);
	memcpy(packed, pPack, sizeof(*pPack));
	codec_encode(packed, sizeof(*pPack), packed_len, password);
	mpqapi_write_file(SAVEFILE_HERO, packed, packed_len);
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

static void pfile_flush(bool bFree)
{
	mpqapi_flush_and_close(bFree);
}

static HANDLE pfile_open_save_archive(unsigned save_num)
{
	return SFileOpenArchive(GetSavePath(save_num).c_str(), 0);
}

void pfile_write_hero()
{
	PkPlayerStruct pkplr;

	if (pfile_open_archive()) {
		PackPlayer(&pkplr, mypnum);
		pfile_encode_hero(&pkplr);
		pfile_flush(!IsMultiGame);
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

/*bool pfile_rename_hero(const char *name_1, const char *name_2)
{
	int i;
	unsigned save_num;
	_uiheroinfo uihero;
	bool found = false;

	SStrCopy(players[i]._pName, name_2, PLR_NAME_LEN);
	pfile_player2hero(&players[0], &uihero, mySaveIdx, gbValidSaveFile);
	pfile_write_hero();
	return true;
}*/

void pfile_flush_W()
{
	pfile_flush(true);
}

static bool pfile_archive_contains_game(HANDLE hsArchive)
{
	HANDLE file;

	if (IsMultiGame)
		return false;

	if (!SFileOpenFileEx(hsArchive, SAVEFILE_GAME, SFILE_OPEN_FROM_MPQ, &file))
		return false;

	SFileCloseFile(file);
	return true;
}

void pfile_ui_set_hero_infos(void (*ui_add_hero_info)(_uiheroinfo *))
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
				ui_add_hero_info(&uihero);
			}
			SFileCloseArchive(archive);
		}
	}
}

/*void pfile_ui_set_class_stats(unsigned int player_class_nr, _uidefaultstats *class_stats)
{
	class_stats->dsStrength = StrengthTbl[player_class_nr];
	class_stats->dsMagic = MagicTbl[player_class_nr];
	class_stats->dsDexterity = DexterityTbl[player_class_nr];
	class_stats->dsVitality = VitalityTbl[player_class_nr];
}*/

bool pfile_ui_create_save(_uiheroinfo* heroinfo)
{
	unsigned save_num;
	HANDLE archive;
	PkPlayerStruct pkplr;

	save_num = heroinfo->hiIdx;
	if (save_num >= MAX_CHARACTERS) {
		for (save_num = 0; save_num < MAX_CHARACTERS; save_num++) {
			archive = pfile_open_save_archive(save_num);
			if (archive == NULL)
				break;
			SFileCloseArchive(archive);
		}
		if (save_num >= MAX_CHARACTERS)
			return false;
	}
	if (!pfile_open_save_mpq(save_num))
		return false;
	heroinfo->hiIdx = save_num;
	static_assert(MAX_CHARACTERS <= UCHAR_MAX, "Save-file index does not fit to _uiheroinfo.");
	mpqapi_remove_hash_entries(pfile_get_file_name);
	CreatePlayer(*heroinfo);
	PackPlayer(&pkplr, 0);
	pfile_encode_hero(&pkplr);
	//pfile_player2hero(&players[0], heroinfo, save_num, false);
	pfile_flush(true);
	return true;
}

static bool GetPermLevelNames(unsigned dwIndex, char (&szPerm)[MAX_PATH])
{
	const char *fmt;

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

static bool GetTempLevelNames(unsigned dwIndex, char (&szTemp)[MAX_PATH])
{
	const char *fmt;

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

bool pfile_get_file_name(unsigned lvl, char (&dst)[MAX_PATH])
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
}

void pfile_ui_delete_save(_uiheroinfo* hero_info)
{
	unsigned save_num;

	save_num = hero_info->hiIdx;
	assert(save_num < MAX_CHARACTERS);
	RemoveFile(GetSavePath(save_num).c_str());
}

static void pfile_read_player_from_save()
{
	HANDLE archive;
	PkPlayerStruct pkplr;

	archive = pfile_open_save_archive(mySaveIdx);
	if (archive == NULL)
		app_fatal("Unable to open save file archive");
	if (!pfile_read_hero(archive, &pkplr))
		app_fatal("Unable to read save file");

	UnPackPlayer(&pkplr, 0); // mypnum
	gbValidSaveFile = pfile_archive_contains_game(archive);
	SFileCloseArchive(archive);
	guNextSaveTc = SDL_GetTicks() + PFILE_SAVE_INTERVAL;
}

void pfile_read_hero_from_save()
{
	pfile_read_player_from_save();
	mypnum = 0;
}

void GetTempLevelName(char (&szTemp)[MAX_PATH])
{
	GetTempLevelNames(currLvl._dLevelIdx, szTemp);
}

void GetPermLevelName(char (&szPerm)[MAX_PATH])
{
	bool has_file;

	GetTempLevelName(szPerm);
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");

	has_file = mpqapi_has_file(szPerm);
	pfile_flush(true);
	if (!has_file) {
		GetPermLevelNames(currLvl._dLevelIdx, szPerm);
	}
}

void pfile_remove_temp_files()
{
	if (!IsMultiGame) {
		if (!pfile_open_archive())
			app_fatal("Unable to open file archive");
		mpqapi_remove_hash_entries(GetTempLevelNames);
		pfile_flush(true);
	}
}

void pfile_rename_temp_to_perm()
{
	unsigned dwIndex;
	bool bResult;
	char szTemp[MAX_PATH];
	char szPerm[MAX_PATH];

	assert(!IsMultiGame);
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");

	dwIndex = 0;
	while (GetTempLevelNames(dwIndex, szTemp)) {
		bResult = GetPermLevelNames(dwIndex, szPerm);
		assert(bResult);
		dwIndex++;
		if (mpqapi_has_file(szTemp)) {
			if (mpqapi_has_file(szPerm))
				mpqapi_remove_hash_entry(szPerm);
			mpqapi_rename(szTemp, szPerm);
		}
	}
	assert(!GetPermLevelNames(dwIndex, szPerm));
	pfile_flush(true);
}

void pfile_write_save_file(const char *pszName, BYTE *pbData, DWORD dwLen, DWORD qwLen)
{
	{
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		codec_encode(pbData, dwLen, qwLen, password);
	}
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");
	mpqapi_write_file(pszName, pbData, qwLen);
	pfile_flush(true);
}

void pfile_delete_save_file(const char *pszName)
{
	if (!pfile_open_archive())
		app_fatal("Unable to open file archive");
	mpqapi_remove_hash_entry(pszName);
	pfile_flush(true);
}

BYTE *pfile_read(const char *pszName)
{
	DWORD nread, len;
	HANDLE archive, save;
	BYTE *buf;

	archive = pfile_open_save_archive(mySaveIdx);
	if (archive == NULL)
		app_fatal("Unable to open save file archive");

	if (!SFileOpenFileEx(archive, pszName, SFILE_OPEN_FROM_MPQ, &save))
		app_fatal("Unable to open save file");

	len = SFileGetFileSize(save);
	if (len == 0)
		app_fatal("Invalid save file");

	buf = DiabloAllocPtr(len);
	if (!SFileReadFile(save, buf, len, &nread))
		app_fatal("Unable to read save file");
	SFileCloseFile(save);
	SFileCloseArchive(archive);

	{
		const char* password = IsMultiGame ? PASSWORD_MULTI : PASSWORD_SINGLE;

		len = codec_decode(buf, len, password);
		if (len == 0) {
			app_fatal("Invalid save file");
		}
	}
	return buf;
}

void pfile_update(bool force_save)
{
	if (IsMultiGame) {
		Uint32 currTc = SDL_GetTicks();
		if (force_save || currTc > guNextSaveTc) {
			guNextSaveTc = currTc + PFILE_SAVE_INTERVAL;
			pfile_write_hero();
		}
	}
}

DEVILUTION_END_NAMESPACE
