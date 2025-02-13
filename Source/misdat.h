/**
 * @file misdat.h
 *
 * Interface of data related to missiles.
 */
#ifndef __MISDAT_H__
#define __MISDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern const MissileData missiledata[];
extern const MisFileData misfiledata[NUM_MFILE + 1];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISDAT_H__ */
