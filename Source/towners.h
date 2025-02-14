/**
 * @file towners.h
 *
 * Interface of functionality for loading and spawning towners.
 */
#ifndef __TOWNERS_H__
#define __TOWNERS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern const int GossipList[STORE_TOWNERS][2];
extern int numtowners;

void InitTowners();
void FreeTownerGFX();
void ProcessTowners();
bool PlrHasStorageItem(int pnum, int item, int* outidx);
void TalkToTowner(int tnum, int pnum);
void SyncTownerQ(int pnum, int idx);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNERS_H__ */
