/**
 * @file debug.h
 *
 * Interface of debug functions.
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG_MODE
void CheckDungeonClear();
extern BYTE* pSquareCel;

void LoadDebugGFX();
void FreeDebugGFX();
void GiveGoldCheat();
void StoresCheat();
void TakeGoldCheat();
void MaxSpellsCheat();
void SetAllSpellsCheat();
void PrintDebugPlayer(bool bNextPlayer);
void PrintDebugQuest();
void GetDebugMonster();
void NextDebugMonster();
#endif

#if DEBUG_MODE || DEV_MODE
void ValidateData();
#endif

#if DEV_MODE
void LogErrorF(const char* type, const char* msg, ...);
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DEBUG_H__ */
