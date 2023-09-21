/**
 * @file town.h
 *
 * Interface of functionality for rendering the town, towners and calling other render routines.
 */
#ifndef __TOWN_H__
#define __TOWN_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HELLFIRE
void OpenNest();
void OpenCrypt();
#endif
void CreateTown();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWN_H__ */
