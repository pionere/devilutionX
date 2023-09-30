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

/** Specifies whether the automap is enabled (_automap_mode). */
extern BYTE gbAutomapflag;
/** Specifies the scale of the automap. */
extern unsigned AutoMapScale;
/* The scale of the normal-automap. */
extern BYTE MiniMapScale;
/* The scale of the mini-automap. */
extern BYTE NormalMapScale;
extern int AutoMapXOfs;
extern int AutoMapYOfs;

/**
 * @brief Initializes the automap.
 */
void InitAutomapOnce();

/**
 * @brief Calculates the cached values of automap which are depending on the scale.
 */
void InitAutomapScale();

/**
 * @brief Initializes the automap of a dungeon level.
 *  1. Loads the mapping between tile IDs and automap shapes.
 *  2. Resets the offsets.
 */
void InitLvlAutomap();

// Whether the automap is being displayed.
bool IsAutomapActive();

/**
 * @brief Display/Hide the automap.
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
/**
 * @brief Renders the automap and the info text on screen.
 */
void DrawAutomap();
/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int x, int y);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __AUTOMAP_H__ */
