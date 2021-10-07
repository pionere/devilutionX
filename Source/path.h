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

int FindPath(bool (*PosOk)(int, int, int), int PosOkArg, int sx, int sy, int dx, int dy, char* path);
bool PathWalkable(int sx, int sy, int pdir);

/* rdata */

extern const char pathxdir[8];
extern const char pathydir[8];
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PATH_H__ */
