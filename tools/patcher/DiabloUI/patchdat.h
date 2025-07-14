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

extern const DeltaFrameData deltaRLHAS[3449];
extern const DeltaFrameData deltaRLHAT[5697];
extern const DeltaFrameData deltaRLHAW[3011];
extern const DeltaFrameData deltaRLHST[12079];
extern const DeltaFrameData deltaRLHBL[2243];
extern const DeltaFrameData deltaRLHFM[1996];
extern const DeltaFrameData deltaRLHLM[3401];
extern const DeltaFrameData deltaRLHHT[1687];
extern const DeltaFrameData deltaRLHQM[2993];
extern const DeltaFrameData deltaRLHWL[3876];
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
