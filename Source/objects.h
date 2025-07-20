/**
 * @file objects.h
 *
 * Interface of object functionality, interaction, spawning, loading, etc.
 */
#ifndef __OBJECTS_H__
#define __OBJECTS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

//extern int objectactive[MAXOBJECTS];
extern int numobjects;
//extern int objectavail[MAXOBJECTS];
extern ObjectStruct objects[MAXOBJECTS];
//extern bool gbInitObjFlag;

void InitObjectGFX();
void FreeObjectGFX();
void InitLvlObjects();
void InitObjects();
int AddObject(int type, int ox, int oy);
void ProcessObjects();
void MonstCheckDoors(int mx, int my);
void ObjChangeMap(int x1, int y1, int x2, int y2 /*, bool hasNewObjPiece*/);
void DisarmObject(int pnum, int oi);
void OperateObject(int pnum, int oi, bool TeleFlag);
bool SyncBloodPass(int pnum, int oi);
void SyncShrineCmd(int pnum, BYTE type, int seed);
// void SyncOpObject(int pnum, int oi);
void SyncOpObject(int oi);
void SyncDoorOpen(int oi);
void SyncDoorClose(int oi);
void SyncTrapDisable(int oi);
//void SyncTrapOpen(int oi);
//void SyncTrapClose(int oi);
void SyncObjectAnim(int oi);
void GetObjectStr(int oi);
#ifdef HELLFIRE
void OpenNakrulRoom();
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJECTS_H__ */
