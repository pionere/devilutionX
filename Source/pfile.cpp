/**
 * @file pfile.cpp
 *
 * Implementation of the save game encoding functionality.
 */
#include <string>

#include "all.h"
#include "paths.h"
#include "../DiabloUI/diabloui.h"
#include "file_util.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef SPAWN
const char *PASSWORD_SINGLE = "adslhfb1";
const char *PASSWORD_MULTI = "lshbkfg1";

#ifdef HELLFIRE
#define SAVE_FILE_FORMAT_SINGLE "spawn%d.hsv"
#define SAVE_FILE_FORMAT_MULTI "share_%d.hsv"
#else
#define SAVE_FILE_FORMAT_SINGLE "spawn%d.sv"
#define SAVE_FILE_FORMAT_MULTI "share_%d.sv"
#endif
#else
const char *PASSWORD_SINGLE = "xrgyrkj1";
const char *PASSWORD_MULTI = "szqnlsk1";

#ifdef HELLFIRE
#define SAVE_FILE_FORMAT_SINGLE "single_%d.hsv"
#define SAVE_FILE_FORMAT_MULTI "hrinfo_%d.drv"
#else
#define SAVE_FILE_FORMAT_SINGLE "single_%d.sv"
#define SAVE_FILE_FORMAT_MULTI "multi_%d.sv"
#endif
#endif

namespace {

std::string GetSavePath(DWORD save_num)
{
	std::string path = GetPrefPath();

	const char* fmt = gbMaxPlayers != 1 ? SAVE_FILE_FORMAT_MULTI : SAVE_FILE_FORMAT_SINGLE;

	char save_file_name[21];
	snprintf(save_file_name, sizeof(save_file_name) / sizeof(char), fmt, save_num);
	path.append(save_file_name);
	return path;
}

} // namespace

/** List of character names for the character selection screen. */
static char hero_names[MAX_CHARACTERS][PLR_NAME_LEN];
BOOL gbValidSaveFile;

static DWORD pfile_get_save_num_from_name(const char *name)
{
	DWORD i;

	for (i = 0; i < MAX_CHARACTERS; i++) {
		if (!strcasecmp(hero_names[i], name))
			break;
	}

	return i;
}

static BOOL pfile_read_hero(HANDLE archive, PkPlayerStruct *pPack)
{
	HANDLE file;
	DWORD dwlen;
	BYTE *buf;

	if (!SFileOpenFileEx(archive, "hero", 0, &file)) {
		return FALSE;
	} else {
		BOOL ret = FALSE;
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

		dwlen = SFileGetFileSize(file, NULL);
		if (dwlen != 0) {
			DWORD read;
			buf = DiabloAllocPtr(dwlen);
			if (SFileReadFile(file, buf, dwlen, &read, NULL)) {
				read = codec_decode(buf, dwlen, password);
				if (read == sizeof(*pPack)) {
					memcpy(pPack, buf, sizeof(*pPack));
					ret = TRUE;
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
	mpqapi_write_file("hero", packed, packed_len);
	mem_free_dbg(packed);
}

static BOOL pfile_open_archive(BOOL update, DWORD save_num)
{
	if (OpenMPQ(GetSavePath(save_num).c_str(), save_num))
		return TRUE;

	return FALSE;
}

static void pfile_flush(BOOL is_single_player, DWORD save_num)
{
	mpqapi_flush_and_close(GetSavePath(save_num).c_str(), is_single_player, save_num);
}

/**
 * @param showFixedMsg Display a dialog if a save file was corrected (deprecated)
 */
static HANDLE pfile_open_save_archive(BOOL *showFixedMsg, DWORD save_num)
{
	HANDLE archive;

	if (SFileOpenArchive(GetSavePath(save_num).c_str(), 0x7000, FS_PC, &archive))
		return archive;
	return NULL;
}

static void pfile_SFileCloseArchive(HANDLE hsArchive)
{
	SFileCloseArchive(hsArchive);
}

void pfile_write_hero()
{
	DWORD save_num;
	PkPlayerStruct pkplr;

	save_num = pfile_get_save_num_from_name(plr[myplr]._pName);
	if (pfile_open_archive(TRUE, save_num)) {
		PackPlayer(&pkplr, myplr, gbMaxPlayers == 1);
		pfile_encode_hero(&pkplr);
		pfile_flush(gbMaxPlayers == 1, save_num);
	}
}

BOOL pfile_create_player_description(char *dst, DWORD len)
{
	char desc[128];
	_uiheroinfo uihero;

	myplr = 0;
	pfile_read_player_from_save();
	game_2_ui_player(plr, &uihero, gbValidSaveFile);
	UiSetupPlayerInfo(gszHero, &uihero, GAME_ID);

	if (dst != NULL && len) {
		if (UiCreatePlayerDescription(&uihero, GAME_ID, desc) == 0)
			return FALSE;
		SStrCopy(dst, desc, len);
	}
	return TRUE;
}

BOOL pfile_rename_hero(const char *name_1, const char *name_2)
{
	int i;
	DWORD save_num;
	_uiheroinfo uihero;
	BOOL found = FALSE;

	if (pfile_get_save_num_from_name(name_2) == MAX_CHARACTERS) {
		for (i = 0; i != MAX_PLRS; i++) {
			if (!strcasecmp(name_1, plr[i]._pName)) {
				found = TRUE;
				break;
			}
		}
	}

	if (!found)
		return FALSE;
	save_num = pfile_get_save_num_from_name(name_1);
	if (save_num == MAX_CHARACTERS)
		return FALSE;

	SStrCopy(hero_names[save_num], name_2, PLR_NAME_LEN);
	SStrCopy(plr[i]._pName, name_2, PLR_NAME_LEN);
	if (!strcasecmp(gszHero, name_1))
		SStrCopy(gszHero, name_2, sizeof(gszHero));
	game_2_ui_player(plr, &uihero, gbValidSaveFile);
	UiSetupPlayerInfo(gszHero, &uihero, GAME_ID);
	pfile_write_hero();
	return TRUE;
}

void pfile_flush_W()
{
	pfile_flush(TRUE, pfile_get_save_num_from_name(plr[myplr]._pName));
}

static char pfile_get_player_class(unsigned int player_class_nr)
{
	char pc_class;

	if (player_class_nr == UI_WARRIOR)
		pc_class = PC_WARRIOR;
	else if (player_class_nr == UI_ROGUE)
		pc_class = PC_ROGUE;
#ifdef HELLFIRE
	else if (player_class_nr == UI_MONK)
		pc_class = PC_MONK;
	else if (player_class_nr == UI_BARD)
		pc_class = PC_BARD;
	else if (player_class_nr == UI_BARBARIAN)
		pc_class = PC_BARBARIAN;
#endif
	else
		pc_class = PC_SORCERER;
	return pc_class;
}


static BYTE game_2_ui_class(const PlayerStruct *p)
{
	BYTE uiclass;
	if (p->_pClass == PC_WARRIOR)
		uiclass = UI_WARRIOR;
	else if (p->_pClass == PC_ROGUE)
		uiclass = UI_ROGUE;
#ifdef HELLFIRE
	else if (p->_pClass == PC_MONK)
		uiclass = UI_MONK;
	else if (p->_pClass == PC_BARD)
		uiclass = UI_BARD;
	else if (p->_pClass == PC_BARBARIAN)
		uiclass = UI_BARBARIAN;
#endif
	else
		uiclass = UI_SORCERER;

	return uiclass;
}

void game_2_ui_player(const PlayerStruct *p, _uiheroinfo *heroinfo, BOOL bHasSaveFile)
{
	memset(heroinfo, 0, sizeof(*heroinfo));
	SStrCopy(heroinfo->name, p->_pName, sizeof(heroinfo->name));
	heroinfo->level = p->_pLevel;
	heroinfo->heroclass = game_2_ui_class(p);
	heroinfo->strength = p->_pStrength;
	heroinfo->magic = p->_pMagic;
	heroinfo->dexterity = p->_pDexterity;
	heroinfo->vitality = p->_pVitality;
	heroinfo->gold = p->_pGold;
	heroinfo->hassaved = bHasSaveFile;
	heroinfo->herorank = p->pDiabloKillLevel;
#ifdef SPAWN
	heroinfo->spawned = TRUE;
#else
	heroinfo->spawned = FALSE;
#endif
}

BOOL pfile_ui_set_hero_infos(BOOL(*ui_add_hero_info)(_uiheroinfo *))
{
	DWORD i;
	BOOL showFixedMsg;

	memset(hero_names, 0, sizeof(hero_names));

	showFixedMsg = TRUE;
	for (i = 0; i < MAX_CHARACTERS; i++) {
		PkPlayerStruct pkplr;
		HANDLE archive = pfile_open_save_archive(&showFixedMsg, i);
		if (archive) {
			if (pfile_read_hero(archive, &pkplr)) {
				_uiheroinfo uihero;
				copy_str(hero_names[i], pkplr.pName)
				UnPackPlayer(&pkplr, 0, FALSE);
				game_2_ui_player(plr, &uihero, pfile_archive_contains_game(archive, i));
				ui_add_hero_info(&uihero);
			}
			pfile_SFileCloseArchive(archive);
		}
	}

	return TRUE;
}

BOOL pfile_archive_contains_game(HANDLE hsArchive, DWORD save_num)
{
	HANDLE file;

	if (gbMaxPlayers != 1)
		return FALSE;

	if (!SFileOpenFileEx(hsArchive, "game", 0, &file))
		return FALSE;

	SFileCloseFile(file);
	return TRUE;
}

BOOL pfile_ui_set_class_stats(unsigned int player_class_nr, _uidefaultstats *class_stats)
{
	int c;

	c = pfile_get_player_class(player_class_nr);
	class_stats->strength = StrengthTbl[c];
	class_stats->magic = MagicTbl[c];
	class_stats->dexterity = DexterityTbl[c];
	class_stats->vitality = VitalityTbl[c];
	return TRUE;
}

BOOL pfile_ui_save_create(_uiheroinfo *heroinfo)
{
	DWORD save_num;
	char cl;
	PkPlayerStruct pkplr;

	save_num = pfile_get_save_num_from_name(heroinfo->name);
	if (save_num == MAX_CHARACTERS) {
		for (save_num = 0; save_num < MAX_CHARACTERS; save_num++) {
			if (!hero_names[save_num][0])
				break;
		}
		if (save_num == MAX_CHARACTERS)
			return FALSE;
	}
	if (!pfile_open_archive(FALSE, save_num))
		return FALSE;
	mpqapi_remove_hash_entries(pfile_get_file_name);
	copy_str(hero_names[save_num], heroinfo->name);
	cl = pfile_get_player_class(heroinfo->heroclass);
	CreatePlayer(0, cl);
	copy_str(plr[0]._pName, heroinfo->name)
	PackPlayer(&pkplr, 0, TRUE);
	pfile_encode_hero(&pkplr);
	game_2_ui_player(&plr[0], heroinfo, FALSE);
	pfile_flush(TRUE, save_num);
	return TRUE;
}

BOOL pfile_get_file_name(DWORD lvl, char (&dst)[MAX_PATH])
{
	const char *fmt;

	if (gbMaxPlayers != 1) {
		if (lvl != 0)
			return FALSE;
		fmt = "hero";
	} else {
		if (lvl < 17)
			fmt = "perml%02d";
		else if (lvl < 34) {
			lvl -= 17;
			fmt = "perms%02d";
		} else if (lvl == 34)
			fmt = "game";
		else if (lvl == 35)
			fmt = "hero";
		else
			return FALSE;
	}
	snprintf(dst, sizeof(dst), fmt, lvl);
	return TRUE;
}

BOOL pfile_delete_save(_uiheroinfo *hero_info)
{
	DWORD save_num;

	save_num = pfile_get_save_num_from_name(hero_info->name);
	if (save_num < MAX_CHARACTERS) {
		hero_names[save_num][0] = '\0';
		RemoveFile(GetSavePath(save_num).c_str());
	}
	return TRUE;
}

void pfile_read_player_from_save()
{
	HANDLE archive;
	DWORD save_num;
	PkPlayerStruct pkplr;

	save_num = pfile_get_save_num_from_name(gszHero);
	archive = pfile_open_save_archive(NULL, save_num);
	if (archive == NULL)
		app_fatal("Unable to open archive");
	if (!pfile_read_hero(archive, &pkplr))
		app_fatal("Unable to load character");

	UnPackPlayer(&pkplr, myplr, FALSE);
	gbValidSaveFile = pfile_archive_contains_game(archive, save_num);
	pfile_SFileCloseArchive(archive);
}

void GetTempLevelNames(char (&szTemp)[MAX_PATH])
{
	if (setlevel)
		snprintf(szTemp, sizeof(szTemp), "temps%02d", setlvlnum);
	else
		snprintf(szTemp, sizeof(szTemp), "templ%02d", currlevel);
}

void GetPermLevelNames(char (&szPerm)[MAX_PATH])
{
	DWORD save_num;
	BOOL has_file;

	save_num = pfile_get_save_num_from_name(plr[myplr]._pName);
	GetTempLevelNames(szPerm);
	if (!pfile_open_archive(FALSE, save_num))
		app_fatal("Unable to read to save file archive");

	has_file = mpqapi_has_file(szPerm);
	pfile_flush(TRUE, save_num);
	if (!has_file) {
		if (setlevel)
			snprintf(szPerm, sizeof(szPerm), "perms%02d", setlvlnum);
		else
			snprintf(szPerm, sizeof(szPerm), "perml%02d", currlevel);
	}
}

void pfile_get_game_name(char (&dst)[MAX_PATH])
{
	copy_cstr(dst, "game");
}

static BOOL GetPermSaveNames(DWORD dwIndex, char (&szPerm)[MAX_PATH])
{
	const char *fmt;

	if (dwIndex < 17)
		fmt = "perml%02d";
	else if (dwIndex < 34) {
		dwIndex -= 17;
		fmt = "perms%02d";
	} else
		return FALSE;

	snprintf(szPerm, sizeof(szPerm), fmt, dwIndex);
	return TRUE;
}

static BOOL GetTempSaveNames(DWORD dwIndex, char (&szTemp)[MAX_PATH])
{
	const char *fmt;

	if (dwIndex < 17)
		fmt = "templ%02d";
	else if (dwIndex < 34) {
		dwIndex -= 17;
		fmt = "temps%02d";
	} else
		return FALSE;

	snprintf(szTemp, sizeof(szTemp), fmt, dwIndex);
	return TRUE;
}

void pfile_remove_temp_files()
{
	if (gbMaxPlayers == 1) {
		DWORD save_num = pfile_get_save_num_from_name(plr[myplr]._pName);
		if (!pfile_open_archive(FALSE, save_num))
			app_fatal("Unable to write to save file archive");
		mpqapi_remove_hash_entries(GetTempSaveNames);
		pfile_flush(TRUE, save_num);
	}
}

void pfile_rename_temp_to_perm()
{
	DWORD dwChar, dwIndex;
	BOOL bResult;
	char szTemp[MAX_PATH];
	char szPerm[MAX_PATH];

	dwChar = pfile_get_save_num_from_name(plr[myplr]._pName);
	assert(dwChar < MAX_CHARACTERS);
	assert(gbMaxPlayers == 1);
	if (!pfile_open_archive(FALSE, dwChar))
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
	pfile_flush(TRUE, dwChar);
}

void pfile_write_save_file(const char *pszName, BYTE *pbData, DWORD dwLen, DWORD qwLen)
{
	DWORD save_num;

	save_num = pfile_get_save_num_from_name(plr[myplr]._pName);
	{
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

		codec_encode(pbData, dwLen, qwLen, password);
	}
	if (!pfile_open_archive(FALSE, save_num))
		app_fatal("Unable to write so save file archive");
	mpqapi_write_file(pszName, pbData, qwLen);
	pfile_flush(TRUE, save_num);
}

BYTE *pfile_read(const char *pszName, DWORD *pdwLen)
{
	DWORD save_num, nread;
	HANDLE archive, save;
	BYTE *buf;

	save_num = pfile_get_save_num_from_name(plr[myplr]._pName);
	archive = pfile_open_save_archive(NULL, save_num);
	if (archive == NULL)
		app_fatal("Unable to open save file archive");

	if (!SFileOpenFileEx(archive, pszName, 0, &save))
		app_fatal("Unable to open save file");

	*pdwLen = SFileGetFileSize(save, NULL);
	if (*pdwLen == 0)
		app_fatal("Invalid save file");

	buf = (BYTE *)DiabloAllocPtr(*pdwLen);
	if (!SFileReadFile(save, buf, *pdwLen, &nread, NULL))
		app_fatal("Unable to read save file");
	SFileCloseFile(save);
	pfile_SFileCloseArchive(archive);

	{
		const char *password = gbMaxPlayers != 1 ? PASSWORD_MULTI : PASSWORD_SINGLE;

		*pdwLen = codec_decode(buf, *pdwLen, password);
		if (*pdwLen == 0) {
			app_fatal("Invalid save file");
		}
	}
	return buf;
}

void pfile_update(BOOL force_save)
{
	// BUGFIX: these tick values should be treated as unsigned to handle overflows correctly
	static int save_prev_tc;

	if (gbMaxPlayers != 1) {
		int tick = SDL_GetTicks();
		if (force_save || tick - save_prev_tc > 60000) {
			save_prev_tc = tick;
			pfile_write_hero();
		}
	}
}

DEVILUTION_END_NAMESPACE
