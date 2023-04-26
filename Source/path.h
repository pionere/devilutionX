/**
 * @file path.h
 *
 * Interface of the path finding algorithms.
 */
#ifndef __PATH_H__
#define __PATH_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

int FindPath(bool (*PosOk)(int, int, int), int PosOkArg, int sx, int sy, int dx, int dy, int8_t* path);
bool PathWalkable(int sx, int sy, int pdir);

/* rdata */

extern const int8_t pathxdir[8];
extern const int8_t pathydir[8];
/** Maps from facing direction to path-direction. */
extern const BYTE dir2pdir[NUM_DIRS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PATH_H__ */
