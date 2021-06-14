/**
 * @file drlg_l4.h
 *
 * Interface of the hell level generation algorithms.
 */
#ifndef __DRLG_L4_H__
#define __DRLG_L4_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define DIAB_QUAD_1X	(setpc_x + 3)
#define DIAB_QUAD_1Y	(setpc_y + 3)
#define DIAB_QUAD_2X	(28 - setpc_x)
#define DIAB_QUAD_2Y	(setpc_y)
#define DIAB_QUAD_3X	(setpc_x)
#define DIAB_QUAD_3Y	(28 - setpc_y)
#define DIAB_QUAD_4X	(29 - setpc_x)
#define DIAB_QUAD_4Y	(29 - setpc_y)

// position of the circle in front of Lazurus (single player)
#define LAZ_CIRCLE_X	DBORDERX + 19
#define LAZ_CIRCLE_Y	DBORDERY + 30

void CreateL4Dungeon(int entry);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLG_L4_H__ */
