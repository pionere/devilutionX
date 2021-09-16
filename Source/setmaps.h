/**
 * @file setmaps.cpp
 *
 * Interface of functionality for the special quest dungeons.
 */
#ifndef __SETMAPS_H__
#define __SETMAPS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Load a quest map, the given map is specified via the global setlvlnum
 */
void LoadSetMap();

/* rdata */
extern const char *const quest_level_names[];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SETMAPS_H__ */
