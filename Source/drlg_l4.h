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

#define DQUAD_ROOM_SIZE 14
#define DQUAD_ROOM      (DQUAD_ROOM_SIZE - 2)

#define DIAB_QUAD_1W 6
#define DIAB_QUAD_1H 6
//(setpc_x + 3)
#define DIAB_QUAD_1X (setpc_x + (DQUAD_ROOM - DIAB_QUAD_1W) / 2)
//(setpc_y + 3)
#define DIAB_QUAD_1Y (setpc_y + (DQUAD_ROOM - DIAB_QUAD_1H) / 2)
#define DIAB_QUAD_2W 11
#define DIAB_QUAD_2H 12
//(28 - setpc_x)
#define DIAB_QUAD_2X (DMAXX - 1 - (DQUAD_ROOM - DIAB_QUAD_2W) / 2 - DIAB_QUAD_2W - setpc_x)
//(setpc_y)
#define DIAB_QUAD_2Y (setpc_y + (DQUAD_ROOM - DIAB_QUAD_2H) / 2)
#define DIAB_QUAD_3W 11
#define DIAB_QUAD_3H 11
//(setpc_x)
#define DIAB_QUAD_3X (setpc_x + (DQUAD_ROOM - DIAB_QUAD_3W) / 2)
//(39 - 11 - setpc_y)
#define DIAB_QUAD_3Y (DMAXY - 1 - (DQUAD_ROOM - 11) / 2 - DIAB_QUAD_3H - setpc_y)
#define DIAB_QUAD_4W 9
#define DIAB_QUAD_4H 9
//(29 - setpc_x)
#define DIAB_QUAD_4X (DMAXX - 1 - (DQUAD_ROOM - DIAB_QUAD_4W) / 2 - DIAB_QUAD_4W - setpc_x)
//(29 - setpc_y)
#define DIAB_QUAD_4Y (DMAXY - 1 - (DQUAD_ROOM - DIAB_QUAD_4H) / 2 - DIAB_QUAD_4H - setpc_y)

// position of the circle in front of Lazarus (single player)
#define LAZ_CIRCLE_X (DBORDERX + 19)
#define LAZ_CIRCLE_Y (DBORDERY + 30)

void CreateL4Dungeon();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLG_L4_H__ */
