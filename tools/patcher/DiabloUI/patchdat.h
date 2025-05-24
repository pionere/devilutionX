/**
 * @file patchdat.h
 *
 * Interface of data related to patches.
 */
#ifndef __PATCHDAT_H__
#define __PATCHDAT_H__

#include "../all.h"

DEVILUTION_BEGIN_NAMESPACE

typedef struct DeltaFrameData {
	BYTE dfFrameNum;
	BYTE dfx;
	BYTE dfy;
	BYTE color;
} DeltaFrameData;

#ifdef __cplusplus
extern "C" {
#endif

extern const DeltaFrameData deltaRLMAT[6037];
extern const DeltaFrameData deltaRMHAT[5043];
extern const DeltaFrameData deltaRMMAT[5280];
extern const DeltaFrameData deltaRMBFM[1153];
extern const DeltaFrameData deltaRMBLM[2470];
extern const DeltaFrameData deltaRMBQM[2123];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PATCHDAT_H__ */
