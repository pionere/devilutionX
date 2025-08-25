/**
 * @file lighting.h
 *
 * Interface of light and vision.
 */
#ifndef __LIGHTING_H__
#define __LIGHTING_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define MAXDARKNESS     15
#define NUM_COLOR_TRNS  (MAXDARKNESS + 1 + 3 + 0)
#define COLOR_TRN_RED   MAXDARKNESS + 1
#define COLOR_TRN_GRAY  MAXDARKNESS + 2
#define COLOR_TRN_CORAL MAXDARKNESS + 3
// #define COLOR_TRN_UNIQ  MAXDARKNESS + 4
extern BYTE ColorTrns[NUM_COLOR_TRNS][NUM_COLORS];

void InitLighting();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LIGHTING_H__ */
