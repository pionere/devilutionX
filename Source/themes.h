/**
 * @file themes.h
 *
 * Interface of the theme room placing algorithms.
 */
#ifndef __THEMES_H__
#define __THEMES_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int numthemes;
extern int zharlib;
extern ThemeStruct themes[MAXTHEMES];

void InitLvlThemes();
void InitThemes();

/**
 * @brief HoldThemeRooms marks theme rooms as populated.
 */
void HoldThemeRooms();

/**
 * CreateThemeRooms adds thematic elements to rooms.
 */
void CreateThemeRooms();

#ifdef __cplusplus
}
#endif
DEVILUTION_END_NAMESPACE

#endif /* __THEMES_H__ */
