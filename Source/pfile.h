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
extern uint32_t guLastSaveTurn;

void pfile_read_hero();
//bool pfile_rename_hero(const char* name_1, const char* name_2);
/*
 * Write save file (game or level) from gsDeltaData.ddBuffer
 */
void pfile_write_save_file(bool full, DWORD dwLen);
/*
 * Delete save file (temp-levels)
 */
void pfile_delete_save_file();
/*
 * Read save file (game or level) into gsDeltaData.ddBuffer
 */
void pfile_read_save_file(bool full);
void pfile_update(bool force_save);
void pfile_close();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PFILE_H__ */
