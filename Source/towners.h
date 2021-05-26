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

extern TownerStruct towners[MAX_TOWNERS];
extern int numtowners;

void InitTowners();
void FreeTownerGFX();
void ProcessTowners();
bool PlrHasItem(int pnum, int item, int *outidx);
void TalkToTowner(int tnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNERS_H__ */
