/**
 * @file sync.h
 *
 * Interface of functionality for syncing game state with other players.
 */
#ifndef __SYNC_H__
#define __SYNC_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

unsigned sync_all_monsters(const BYTE* pbBuf, unsigned dwMaxLen);
//void sync_update(int pnum, const TSyncHeader* pbBuf);
void InitSync();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SYNC_H__ */
