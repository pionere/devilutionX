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

extern int objectactive[MAXOBJECTS];
extern int nobjects;
extern int objectavail[MAXOBJECTS];
extern ObjectStruct object[MAXOBJECTS];
//extern bool gbInitObjFlag;

void InitObjectGFX();
void FreeObjectGFX();
void AddL1Objs(int x1, int y1, int x2, int y2);
void AddL2Objs(int x1, int y1, int x2, int y2);
void InitObjects();
void SetMapObjects(BYTE *pMap);
void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v);
void AddMushPatch();
void AddSlainHero();
#ifdef HELLFIRE
void AddHBooks(int bookidx, int ox, int oy);
void SetupHBook(int oi, int bookidx);
#endif
int AddObject(int type, int ox, int oy);
void ProcessObjects();
void ObjSetMicro(int dx, int dy, int pn);
void RedoPlayerVision();
void MonstCheckDoors(int mx, int my);
void ObjChangeMap(int x1, int y1, int x2, int y2);
void ObjChangeMapResync(int x1, int y1, int x2, int y2);
void DisarmObject(int pnum, int oi);
void OperateObject(int pnum, int oi, bool TeleFlag);
void SyncOpObject(int pnum, int oi);
void SyncDoorOpen(int oi);
void SyncDoorClose(int oi);
void SyncTrapDisable(int oi);
void SyncTrapOpen(int oi);
void SyncTrapClose(int oi);
void SyncChestClose(int oi);
void SyncObjectAnim(int oi);
void GetObjectStr(int oi);
#ifdef HELLFIRE
void OpenUberRoom();
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJECTS_H__ */
