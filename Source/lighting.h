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

#define NO_LIGHT			MAXLIGHTS

extern BYTE visionactive[MAXVISION];
extern LightListStruct VisionList[MAXVISION];
extern BYTE lightactive[MAXLIGHTS];
extern LightListStruct LightList[MAXLIGHTS];
extern int numlights;
extern int numvision;
#ifdef _DEBUG
extern char lightmax;
#define LIGHTMAX lightmax
#else
#define LIGHTMAX 15
#endif
#define NUM_LIGHT_TRNS	LIGHTMAX + 12

#define LIGHTIDX_RED	LIGHTMAX + 1
#define LIGHTIDX_GRAY	LIGHTMAX + 2
#define LIGHTIDX_CORAL	LIGHTMAX + 3
#define LIGHTIDX_UNIQ	LIGHTMAX + 4
extern BYTE LightTrns[NUM_LIGHT_TRNS][256];

void DoLighting(int nXPos, int nYPos, int nRadius, unsigned lnum);
void DoUnVision(int nXPos, int nYPos, int nRadius);
void DoVision(int nXPos, int nYPos, int nRadius, bool local);
void MakeLightTable();
#ifdef _DEBUG
void ToggleLighting();
#endif
void InitLightMax();
void InitLighting();
unsigned AddLight(int x, int y, int r);
void AddUnLight(unsigned lnum);
void ChangeLightRadius(unsigned lnum, int r);
void ChangeLightXY(unsigned lnum, int x, int y);
void ChangeLightOff(unsigned lnum, int xoff, int yoff);
void ChangeLightXYOff(unsigned lnum, int x, int y);
void CondChangeLightOff(unsigned lnum, int xoff, int yoff);
void ChangeLight(unsigned lnum, int x, int y, int r);
void ProcessLightList();
void SavePreLighting();
void InitVision();
int AddVision(int x, int y, int r, bool mine);
void AddUnVision(unsigned vnum);
void ChangeVisionRadius(unsigned vnum, int r);
void ChangeVisionXY(unsigned vnum, int x, int y);
void ProcessVisionList();
void lighting_color_cycling();

/* rdata */

extern const char CrawlTable[2749];
extern const int CrawlNum[19];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LIGHTING_H__ */
