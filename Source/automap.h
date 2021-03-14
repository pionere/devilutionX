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

/** Specifies whether the automap is enabled. */
extern BOOL automapflag;
/** Tracks the explored areas of the map. */
extern BOOLEAN automapview[DMAXX][DMAXY];
/** Specifies the scale of the automap. */
extern unsigned AutoMapScale;
extern int AutoMapXOfs;
extern int AutoMapYOfs;

/**
 * @brief Initializes the automap.
 */
void InitAutomapOnce();

/**
 * @brief Loads the mapping between tile IDs and automap shapes.
 */
void InitAutomap();

/**
 * @brief Displays/Hides the automap.
 */
void ToggleAutomap();

/**
 * @brief Scrolls the automap upwards.
 */
void AutomapUp();

/**
 * @brief Scrolls the automap downwards.
 */
void AutomapDown();

/**
 * @brief Scrolls the automap leftwards.
 */
void AutomapLeft();

/**
 * @brief Scrolls the automap rightwards.
 */
void AutomapRight();

/**
 * @brief Increases the zoom level of the automap.
 */
void AutomapZoomIn();

/**
 * @brief Decreases the zoom level of the automap.
 */
void AutomapZoomOut();
void DrawAutomap();
/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int x, int y);

/**
 * @brief Resets the zoom level of the automap.
 */
void AutomapZoomReset();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __AUTOMAP_H__ */
