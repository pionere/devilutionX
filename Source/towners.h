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

extern TownerStruct towner[MAX_TOWNERS];
extern int numtowners;

void InitTowners();
void FreeTownerGFX();
void ProcessTowners();
BOOL PlrHasItem(int pnum, int item, int *outidx);
void TalkToTowner(int pnum, int tnum);

/* data */

extern int Qtalklist[STORE_TOWNERS][NUM_QUESTS];
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNERS_H__ */
