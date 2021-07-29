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

bool gbValidSaveFile;

static const char *PASSWORD_SINGLE = "xrgyrkj1";
static const char *PASSWORD_MULTI = "szqnlsk1";
/** List of character names for the character selection screen. */
static char hero_names[MAX_CHARACTERS][PLR_NAME_LEN];

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

	const char* fmt = gbMaxPlayers != 1 ? SAVE_FILE_FORMAT_MULTI : SAVE_FILE_FORMAT_SINGLE;
	static_assert(MAX_CHARACTERS < 100, "Name of the save-file does not fit to the temporary buffer.");
	char save_file_name[std::max(sizeof(SAVE_FILE_FORMAT_MULTI), sizeof(SAVE_FILE_FORMAT_SINGLE))];
	snprintf(save_file_name, sizeof(save_file_name), fmt, save_num);
	path.append(save_file_name);
	return path;
}

static unsigned pfile_get_save_num_from_name(const char *name)
{
	unsigned i;

	for (i = 0; i < MAX_CHARACTERS; i++) {
		if (strcasecmp(hero_names[i], name) == 0)
			break;
	}

	return i;
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
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

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
			if (buf != NULL)
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
	const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

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
	unsigned save_num;

	save_num = pfile_get_save_num_from_name(myplr._pName);
	assert(save_num < MAX_CHARACTERS);
	return pfile_open_save_mpq(save_num);
}

static void pfile_flush(bool is_single_player)
{
	mpqapi_flush_and_close(is_single_player);
}

static HANDLE pfile_open_save_archive(unsigned save_num)
{
	HANDLE archive;

	if (SFileOpenArchive(GetSavePath(save_num).c_str(), 0, &archive))
		return archive;
	return NULL;
}

static void pfile_SFileCloseArchive(HANDLE hsArchive)
{
	SFileCloseArchive(hsArchive);
}

void pfile_write_hero()
{
	PkPlayerStruct pkplr;

	if (pfile_open_archive()) {
		PackPlayer(&pkplr, mypnum);
		pfile_encode_hero(&pkplr);
		pfile_flush(gbMaxPlayers == 1);
	}
}

static void game_2_ui_player(const PlayerStruct *p, _uiheroinfo *heroinfo, bool bHasSaveFile)
{
	memset(heroinfo, 0, sizeof(*heroinfo));
	SStrCopy(heroinfo->name, p->_pName, sizeof(heroinfo->name));
	heroinfo->level = p->_pLevel;
	heroinfo->heroclass = p->_pClass;
	heroinfo->strength = p->_pStrength;
	heroinfo->magic = p->_pMagic;
	heroinfo->dexterity = p->_pDexterity;
	heroinfo->vitality = p->_pVitality;
	heroinfo->hassaved = bHasSaveFile;
	heroinfo->herorank = p->_pDiabloKillLevel;
}

/*bool pfile_rename_hero(const char *name_1, const char *name_2)
{
	int i;
	unsigned save_num;
	_uiheroinfo uihero;
	bool found = false;

	if (pfile_get_save_num_from_name(name_2) == MAX_CHARACTERS) {
		for (i = 0; i != MAX_PLRS; i++) {
			if (!strcasecmp(name_1, players[i]._pName)) {
				found = true;
				break;
			}
		}
	}

	if (!found)
		return false;
	save_num = pfile_get_save_num_from_name(name_1);
	if (save_num == MAX_CHARACTERS)
		return false;

	SStrCopy(hero_names[save_num], name_2, PLR_NAME_LEN);
	SStrCopy(players[i]._pName, name_2, PLR_NAME_LEN);
	if (!strcasecmp(gszHero, name_1))
		SStrCopy(gszHero, name_2, sizeof(gszHero));
	game_2_ui_player(&players[0], &uihero, gbValidSaveFile);
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

	if (gbMaxPlayers != 1)
		return false;

	if (!SFileOpenFileEx(hsArchive, SAVEFILE_GAME, SFILE_OPEN_FROM_MPQ, &file))
		return false;

	SFileCloseFile(file);
	return true;
}

void pfile_ui_set_hero_infos(void (*ui_add_hero_info)(_uiheroinfo *))
{
	int i;

	memset(hero_names, 0, sizeof(hero_names));

	for (i = 0; i < MAX_CHARACTERS; i++) {
		PkPlayerStruct pkplr;
		HANDLE archive = pfile_open_save_archive(i);
		if (archive != NULL) {
			if (pfile_read_hero(archive, &pkplr)) {
				_uiheroinfo uihero;
				copy_str(hero_names[i], pkplr.pName);
				UnPackPlayer(&pkplr, 0);
				game_2_ui_player(&players[0], &uihero, pfile_archive_contains_game(archive));
				ui_add_hero_info(&uihero);
			}
			pfile_SFileCloseArchive(archive);
		}
	}
}

void pfile_ui_set_class_stats(unsigned int player_class_nr, _uidefaultstats *class_stats)
{
	class_stats->strength = StrengthTbl[player_class_nr];
	class_stats->magic = MagicTbl[player_class_nr];
	class_stats->dexterity = DexterityTbl[player_class_nr];
	class_stats->vitality = VitalityTbl[player_class_nr];
}

bool pfile_ui_save_create(_uiheroinfo *heroinfo)
{
	unsigned save_num;
	PkPlayerStruct pkplr;

	save_num = pfile_get_save_num_from_name(heroinfo->name);
	if (save_num >= MAX_CHARACTERS) {
		for (save_num = 0; save_num < MAX_CHARACTERS; save_num++) {
			if (!hero_names[save_num][0])
				break;
		}
		if (save_num >= MAX_CHARACTERS)
			return false;
	}
	if (!pfile_open_save_mpq(save_num))
		return false;
	mpqapi_remove_hash_entries(pfile_get_file_name);
	copy_str(hero_names[save_num], heroinfo->name);
	CreatePlayer(0, heroinfo->heroclass);
	copy_str(players[0]._pName, heroinfo->name);
	PackPlayer(&pkplr, 0);
	pfile_encode_hero(&pkplr);
	game_2_ui_player(&players[0], heroinfo, false);
	pfile_flush(true);
	return true;
}

static bool GetPermSaveNames(unsigned dwIndex, char (&szPerm)[MAX_PATH])
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

static bool GetTempSaveNames(unsigned dwIndex, char (&szTemp)[MAX_PATH])
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
	if (gbMaxPlayers != 1) {
		if (lvl != 0)
			return false;
		copy_cstr(dst, SAVEFILE_HERO);
	} else {
		if (lvl < NUM_LEVELS)
			return GetPermSaveNames(lvl, dst);
		if (lvl == NUM_LEVELS)
			copy_cstr(dst, SAVEFILE_GAME);
		else if (lvl == NUM_LEVELS + 1)
			copy_cstr(dst, SAVEFILE_HERO);
		else
			return false;
	}
	return true;
}

void pfile_delete_save(_uiheroinfo *hero_info)
{
	unsigned save_num;

	save_num = pfile_get_save_num_from_name(hero_info->name);
	if (save_num < MAX_CHARACTERS) {
		hero_names[save_num][0] = '\0';
		RemoveFile(GetSavePath(save_num).c_str());
	}
}

static void pfile_read_player_from_save()
{
	HANDLE archive;
	unsigned save_num;
	PkPlayerStruct pkplr;

	save_num = pfile_get_save_num_from_name(gszHero);
	archive = pfile_open_save_archive(save_num);
	if (archive == NULL)
		app_fatal("Unable to open archive");
	if (!pfile_read_hero(archive, &pkplr))
		app_fatal("Unable to load character");

	UnPackPlayer(&pkplr, 0); // mypnum
	gbValidSaveFile = pfile_archive_contains_game(archive);
	pfile_SFileCloseArchive(archive);
}

void pfile_create_player_description()
{
	pfile_read_player_from_save();
	mypnum = 0;
	players[0]._pTeam = 0;
}

void GetTempLevelNames(char (&szTemp)[MAX_PATH])
{
	GetTempSaveNames(currLvl._dLevelIdx, szTemp);
}

void GetPermLevelNames(char (&szPerm)[MAX_PATH])
{
	bool has_file;

	GetTempLevelNames(szPerm);
	if (!pfile_open_archive())
		app_fatal("Unable to read to save file archive");

	has_file = mpqapi_has_file(szPerm);
	pfile_flush(true);
	if (!has_file) {
		GetPermSaveNames(currLvl._dLevelIdx, szPerm);
	}
}

void pfile_remove_temp_files()
{
	if (gbMaxPlayers == 1) {
		if (!pfile_open_archive())
			app_fatal("Unable to write to save file archive");
		mpqapi_remove_hash_entries(GetTempSaveNames);
		pfile_flush(true);
	}
}

void pfile_rename_temp_to_perm()
{
	unsigned dwIndex;
	bool bResult;
	char szTemp[MAX_PATH];
	char szPerm[MAX_PATH];

	assert(gbMaxPlayers == 1);
	if (!pfile_open_archive())
		app_fatal("Unable to write to save file archive");

	dwIndex = 0;
	while (GetTempSaveNames(dwIndex, szTemp)) {
		bResult = GetPermSaveNames(dwIndex, szPerm);
		assert(bResult);
		dwIndex++;
		if (mpqapi_has_file(szTemp)) {
			if (mpqapi_has_file(szPerm))
				mpqapi_remove_hash_entry(szPerm);
			mpqapi_rename(szTemp, szPerm);
		}
	}
	assert(!GetPermSaveNames(dwIndex, szPerm));
	pfile_flush(true);
}

void pfile_write_save_file(const char *pszName, BYTE *pbData, DWORD dwLen, DWORD qwLen)
{
	{
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

		codec_encode(pbData, dwLen, qwLen, password);
	}
	if (!pfile_open_archive())
		app_fatal("Unable to write to save file archive");
	mpqapi_write_file(pszName, pbData, qwLen);
	pfile_flush(true);
}

void pfile_delete_save_file(const char *pszName)
{
	if (!pfile_open_archive())
		app_fatal("Unable to write to save file archive");
	mpqapi_remove_hash_entry(pszName);
	pfile_flush(true);
}

BYTE *pfile_read(const char *pszName)
{
	unsigned save_num;
	DWORD nread, len;
	HANDLE archive, save;
	BYTE *buf;

	save_num = pfile_get_save_num_from_name(myplr._pName);
	archive = pfile_open_save_archive(save_num);
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
	pfile_SFileCloseArchive(archive);

	{
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

		len = codec_decode(buf, len, password);
		if (len == 0) {
			app_fatal("Invalid save file");
		}
	}
	return buf;
}

void pfile_update(bool force_save)
{
	// BUGFIX: these tick values should be treated as unsigned to handle overflows correctly (fixed)
	static DWORD save_prev_tc;

	if (gbMaxPlayers != 1) {
		DWORD tick = SDL_GetTicks();
		if (force_save || tick - save_prev_tc > 60000) {
			save_prev_tc = tick;
			pfile_write_hero();
		}
	}
}

DEVILUTION_END_NAMESPACE
