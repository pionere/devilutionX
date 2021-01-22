/**
 * @file automap.h
 *
 * Interface of the in-game map overlay.
 */
#ifndef __AUTOMAP_H__
#define __AUTOMAP_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL automapflag;
extern BOOLEAN automapview[DMAXX][DMAXY];
extern unsigned AutoMapScale;
extern int AutoMapXOfs;
extern int AutoMapYOfs;

void InitAutomapOnce();
void InitAutomap();
void ToggleAutomap();
void AutomapUp();
void AutomapDown();
void AutomapLeft();
void AutomapRight();
void AutomapZoomIn();
void AutomapZoomOut();
void DrawAutomap();
void SetAutomapView(int x, int y);
void AutomapZoomReset();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __AUTOMAP_H__ */
