/**
 * @file pack.h
 *
 * Interface of functions for minifying player data structure.
 */
#ifndef __PACK_H__
#define __PACK_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void PackPlayer(PkPlayerStruct* pPack, int pnum);
void UnPackPlayer(const PkPlayerStruct* pPack, int pnum);

/* rdata */
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PACK_H__ */
