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
#endif

#if DEBUG_MODE || DEV_MODE
void ValidateData();
#endif

#if DEV_MODE
void LogErrorF(const char* msg, ...);
void LogErrorQ(const char* msg, ...);
void LogDumpQ();
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DEBUG_H__ */
