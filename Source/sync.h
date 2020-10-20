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

extern WORD monster_dists[MAXMONSTERS];
extern WORD monster_prio[MAXMONSTERS];
extern int sync_mnum;
extern int sync_inum;

DWORD sync_all_monsters(const BYTE *pbBuf, DWORD dwMaxLen);
void sync_init_monsters();
BOOL sync_closest_monster(TSyncMonster *symon);
void sync_monster_pos(TSyncMonster *symon, int mnum);
BOOL sync_prio_monster(TSyncMonster *symon);
void SyncPlrInv(TSyncHeader *pHdr);
DWORD sync_update(int pnum, const BYTE *pbBuf);
void sync_monster(int pnum, const TSyncMonster *symon);
void sync_init();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SYNC_H__ */
