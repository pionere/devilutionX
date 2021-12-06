/**
 * @file pfile.h
 *
 * Interface of the save game encoding functionality.
 */
#ifndef __PFILE_H__
#define __PFILE_H__

DEVILUTION_BEGIN_NAMESPACE

#define SAVEFILE_GAME	"game"
#define SAVEFILE_HERO	"hero"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned mySaveIdx;
extern bool gbValidSaveFile;

void pfile_write_hero(bool bFree);
void pfile_read_hero_from_save();
//bool pfile_rename_hero(const char *name_1, const char *name_2);
void pfile_flush(bool bFree);
void pfile_ui_set_hero_infos(void (*ui_add_hero_info)(_uiheroinfo *));
//void pfile_ui_set_class_stats(unsigned int player_class_nr, _uidefaultstats *class_stats);
int pfile_ui_create_save(_uiheroinfo* heroinfo);
void pfile_ui_delete_save(_uiheroinfo* hero_info);
bool pfile_get_file_name(unsigned lvl, char (&dst)[MAX_PATH]);
void GetTempLevelName(char (&szTemp)[MAX_PATH]);
void GetPermLevelName(char (&szPerm)[MAX_PATH]);
void pfile_remove_temp_files();
void pfile_rename_temp_to_perm();
void pfile_write_save_file(const char* pszName, BYTE* pbData, DWORD dwLen);
void pfile_delete_save_file(const char* pszName);
BYTE* pfile_read(const char* pszName);
void pfile_update(bool force_save);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PFILE_H__ */
