/**
 * @file towndat.h
 *
 * Interface of data related to town-patch.
 */
#ifndef __TOWNDAT_H__
#define __TOWNDAT_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

typedef struct DeltaFrameData {
	int dfFrameNum;
	BYTE dfx;
	BYTE dfy;
	BYTE color;
	BYTE align;
} DeltaFrameData;

typedef struct POS {
	BYTE x;
	BYTE y;
} POS;

typedef struct POScol {
	BYTE x;
	BYTE y;
	BYTE col;
	BYTE align;
} POScol;

#ifdef __cplusplus
extern "C" {
#endif

// data to patch CEL
extern const DeltaFrameData deltaGrain[3072];
// data to patch SCEL
extern const POS trans18[47];
extern const POS trans19[814];
extern const POScol cols19[5];
extern const POS trans20[2244];
extern const POScol cols20[23];
extern const POS trans21[650];
extern const POScol cols21[31];
extern const POS trans22[92];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNDAT_H__ */
