/**
 * @file loadsave.h
 *
 * Interface of save game functionality.
 */
#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void LoadGame();
void SaveGame();
void SaveLevel();
void LoadLevel();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LOADSAVE_H__ */
