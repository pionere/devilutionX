/**
 * @file pfile.h
 *
 * Interface of the save game encoding functionality.
 */
#ifndef __PFILE_H__
#define __PFILE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned mySaveIdx;
extern bool gbValidSaveFile;

void pfile_write_hero(bool bFree);
void pfile_read_hero_from_save();
//bool pfile_rename_hero(const char* name_1, const char* name_2);
void pfile_flush(bool bFree);
//bool pfile_get_file_name(unsigned lvl, char (&dst)[DATA_ARCHIVE_MAX_PATH]);
void pfile_rename_temp_to_perm();
/*
 * Write save file (game or level) from gsDeltaData.ddBuffer
 */
void pfile_write_save_file(bool full, DWORD dwLen);
/*
 * Delete save file (game or temp-levels)
 */
void pfile_delete_save_file(bool full);
/*
 * Read save file (game or level) into gsDeltaData.ddBuffer
 */
void pfile_read_save_file(bool full);
void pfile_update(bool force_save);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PFILE_H__ */
