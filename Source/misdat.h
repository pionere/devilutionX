/**
 * @file misdat.h
 *
 * Interface of data related to missiles.
 */
#ifndef __MISDAT_H__
#define __MISDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#define MIA_ACIDPUD_DELAY   1
#define MIA_ACIDPUD1_LENGTH 4
#define MIA_ACIDSPLA_LENGTH 8
#define MIA_ACIDSPLA_DELAY  1
#define MIA_ARROWS_LENGTH   16
#define MIA_BLODBURS_LENGTH 8
#define MIA_BLODBURS_DELAY  2
#define MIA_BLUEXBK_LENGTH  19
#define MIA_BLUEXBK_DELAY   1
#define MIA_BLUEXFR_LENGTH  19
#define MIA_BLUEXFR_DELAY   1
#define MIA_FIREBA_DELAY    1
#define MIA_FIREWAL_LENGTH  13
#define MIA_FIREWAL1_LENGTH 13
#define MIA_GUARD_LENGTH    15
#define MIA_GUARD2_LENGTH   3
#define MIA_INFERNO_LENGTH  20
#define MIA_INFERNO_DELAY   1
#define MIA_LGHNING_DELAY   1
#define MIA_LGHNING_LENGTH  8
#define MIA_MINILTNG_LENGTH 8
#define MIA_PORTAL_LENGTH   16
#define MIA_SHATTER1_LENGTH 12
#define MIA_SHROUD_LENGTH   12
#define MIA_WIND_LENGTH     12
#define MIA_WIND_DELAY      1

#ifdef __cplusplus
extern "C" {
#endif

extern const MissileData missiledata[];
extern const MisFileData misfiledata[NUM_MFILE];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISDAT_H__ */
