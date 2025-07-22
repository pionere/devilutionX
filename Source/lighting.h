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

#define NO_LIGHT      MAXLIGHTS
#define NO_VISION     MAXVISION
#define MAX_LIGHT_RAD 15

extern BYTE visionactive[MAXVISION];
extern LightListStruct VisionList[MAXVISION];
extern BYTE lightactive[MAXLIGHTS];
extern LightListStruct LightList[MAXLIGHTS];
extern int numlights;
extern int numvision;

#define MAXDARKNESS     15
#define NUM_COLOR_TRNS  MAXDARKNESS + 12
#define COLOR_TRN_RED   MAXDARKNESS + 1
#define COLOR_TRN_GRAY  MAXDARKNESS + 2
#define COLOR_TRN_CORAL MAXDARKNESS + 3
#define COLOR_TRN_UNIQ  MAXDARKNESS + 4
extern BYTE ColorTrns[NUM_COLOR_TRNS][NUM_COLORS];

void TraceLightSource(int x, int y, int r);
void DoUnVision(int nXPos, int nYPos, int nRadius);
void DoVision(int nXPos, int nYPos, int nRadius, bool local);
void InitLighting();
#if DEBUG_MODE
void ToggleLighting();
#endif
void InitLvlLighting();
unsigned AddLight(int x, int y, int r);
void AddUnLight(unsigned lnum);
void ChangeLightRadius(unsigned lnum, int r);
void ChangeLightXY(unsigned lnum, int x, int y);
void ChangeLightScreenOff(unsigned lnum, int xsoff, int ysoff);
void ChangeLightXYOff(unsigned lnum, int x, int y);
void CondChangeLightXY(unsigned lnum, int x, int y);
void CondChangeLightScreenOff(unsigned lnum, int xsoff, int ysoff);
void ChangeLight(unsigned lnum, int x, int y, int r);
void ProcessLightList();
inline void SavePreLighting()
{
	memcpy(dPreLight, dLight, sizeof(dPreLight));
}
inline void LoadPreLighting()
{
	memcpy(dLight, dPreLight, sizeof(dPreLight));
}
void InitLvlVision();
void RedoLightAndVision();
void LightAndVisionDone();
int AddVision(int x, int y, int r, bool mine);
void AddUnVision(unsigned vnum);
void ChangeVisionRadius(unsigned vnum, int r);
void ChangeVisionXY(unsigned vnum, int x, int y);
void ProcessVisionList();
void lighting_update_caves();
void lighting_update_hell();
#ifdef HELLFIRE
void lighting_update_crypt();
void lighting_update_nest();
#endif

/* rdata */

extern const int8_t CrawlTable[1514];
extern const int CrawlNum[16];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LIGHTING_H__ */
