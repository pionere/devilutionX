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
extern BOOL dolighting;
extern int visionid;
extern BYTE *pLightTbl;

void DoLighting(int nXPos, int nYPos, int nRadius, int Lnum);
void DoUnVision(int nXPos, int nYPos, int nRadius);
void DoVision(int nXPos, int nYPos, int nRadius, BOOL doautomap, BOOL visible);
void FreeLightTable();
void InitLightTable();
void MakeLightTable();
#ifdef _DEBUG
void ToggleLighting();
#endif
void InitLightMax();
void InitLighting();
int AddLight(int x, int y, int r);
void AddUnLight(int lnum);
void ChangeLightRadius(int lnum, int r);
void ChangeLightXY(int lnum, int x, int y);
void ChangeLightOff(int lnum, int xoff, int yoff);
void ChangeLightXYOff(int lnum, int x, int y, int xoff, int yoff);
void CondChangeLightOff(int lnum, int xoff, int yoff);
void ChangeLight(int lnum, int x, int y, int r);
void ProcessLightList();
void SavePreLighting();
void InitVision();
int AddVision(int x, int y, int r, BOOL mine);
void ChangeVisionRadius(int vnum, int r);
void ChangeVisionXY(int vnum, int x, int y);
void ProcessVisionList();
void lighting_color_cycling();

/* rdata */

extern const char CrawlTable[2749];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LIGHTING_H__ */
