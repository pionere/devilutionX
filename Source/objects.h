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

extern int trapid;
extern int trapdir;
extern BYTE *pObjCels[40];
extern char ObjFileList[40];
extern int objectactive[MAXOBJECTS];
extern int nobjects;
extern int leverid;
extern int objectavail[MAXOBJECTS];
extern ObjectStruct object[MAXOBJECTS];
extern BOOL InitObjFlag;
extern int numobjfiles;

void InitObjectGFX();
void FreeObjectGFX();
DIABOOL RndLocOk(int xp, int yp);
void InitRndLocObj(int min, int max, int objtype);
void InitRndLocBigObj(int min, int max, int objtype);
void InitRndLocObj5x5(int min, int max, int objtype);
void ClrAllObjects();
void AddTortures();
void AddCandles();
void AddBookLever(int lx1, int ly1, int lx2, int ly2, int x1, int y1, int x2, int y2, int msg);
void InitRndBarrels();
void AddL1Objs(int x1, int y1, int x2, int y2);
#ifdef HELLFIRE
void AddCryptObjs(int x1, int y1, int x2, int y2);
#endif
void AddL2Objs(int x1, int y1, int x2, int y2);
void AddL3Objs(int x1, int y1, int x2, int y2);
DIABOOL WallTrapLocOk(int xp, int yp);
void AddL2Torches();
DIABOOL TorchLocOK(int xp, int yp);
void AddObjTraps();
void AddChestTraps();
void LoadMapObjects(BYTE *pMap, int startx, int starty, int x1, int y1, int w, int h, int leveridx);
void LoadMapObjs(BYTE *pMap, int startx, int starty);
void AddDiabObjs();
#ifdef HELLFIRE
void AddLvl2xBooks(int s);
void AddLvl24Books();
#endif
void AddStoryBooks();
void AddHookedBodies(int freq);
void AddL4Goodies();
void AddLazStand();
void InitObjects();
void SetMapObjects(BYTE *pMap, int startx, int starty);
void DeleteObject_(int oi, int i);
void SetupObject(int oi, int x, int y, int type);
void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v);
void SetBookMsg(int oi, int msg);
void AddL1Door(int oi, int x, int y, int type);
void AddSCambBook(int oi);
void AddChest(int oi, int type);
void AddL2Door(int oi, int x, int y, int type);
void AddL3Door(int oi, int x, int y, int type);
void AddSarc(int oi);
void AddFlameTrap(int oi);
void AddFlameLvr(int oi);
void AddTrap(int oi);
void AddObjLight(int oi, int r);
void AddBarrel(int oi, int type);
void AddShrine(int oi);
void AddBookcase(int oi);
void AddPurifyingFountain(int oi);
void AddArmorStand(int oi);
void AddGoatShrine(int oi);
void AddCauldron(int oi);
void AddMurkyFountain(int oi);
void AddTearFountain(int oi);
void AddDecap(int oi);
void AddVilebook(int oi);
void AddMagicCircle(int oi);
void AddBrnCross(int oi);
void AddBookstand(int oi);
void AddBloodFtn(int oi);
void AddPedistal(int oi);
void AddStoryBook(int oi);
void AddWeaponRack(int oi);
void AddTorturedBody(int oi);
void GetRndObjLoc(int randarea, int *xx, int *yy);
void AddMushPatch();
void AddSlainHero();
#ifdef HELLFIRE
void AddHBooks(int bookidx, int ox, int oy);
void SetupHBook(int oi, int bookidx);
#endif
void AddObject(int ot, int ox, int oy);
void Obj_Light(int oi, int lr);
void Obj_Circle(int oi);
void Obj_StopAnim(int oi);
void Obj_Door(int oi);
void Obj_Sarc(int oi);
void ActivateTrapLine(int ttype, int tid);
void Obj_FlameTrap(int oi);
void Obj_Trap(int oi);
void Obj_BCrossDamage(int oi);
void ProcessObjects();
void ObjSetMicro(int dx, int dy, int pn);
void objects_set_door_piece(int x, int y);
void ObjSetMini(int x, int y, int v);
void ObjL1Special(int x1, int y1, int x2, int y2);
void ObjL2Special(int x1, int y1, int x2, int y2);
void DoorSet(int oi, int dx, int dy);
void RedoPlayerVision();
void OperateL1RDoor(int pnum, int oi, DIABOOL sendmsg);
void OperateL1LDoor(int pnum, int oi, DIABOOL sendmsg);
void OperateL2RDoor(int pnum, int oi, DIABOOL sendmsg);
void OperateL2LDoor(int pnum, int oi, BOOL sendmsg);
void OperateL3RDoor(int pnum, int oi, DIABOOL sendmsg);
void OperateL3LDoor(int pnum, int oi, DIABOOL sendmsg);
void MonstCheckDoors(int m);
void ObjChangeMap(int x1, int y1, int x2, int y2);
void ObjChangeMapResync(int x1, int y1, int x2, int y2);
void OperateL1Door(int pnum, int i, DIABOOL sendmsg);
void OperateLever(int pnum, int oi);
void OperateBook(int pnum, int oi);
void OperateBookLever(int pnum, int oi);
void OperateSChambBk(int pnum, int oi);
void OperateChest(int pnum, int oi, DIABOOL sendmsg);
void OperateMushPatch(int pnum, int oi);
void OperateInnSignChest(int pnum, int oi);
void OperateSlainHero(int pnum, int oi, DIABOOL sendmsg);
void OperateTrapLvr(int oi);
void OperateSarc(int pnum, int oi, DIABOOL sendmsg);
void OperateL2Door(int pnum, int oi, DIABOOL sendmsg);
void OperateL3Door(int pnum, int oi, DIABOOL sendmsg);
void OperatePedistal(int pnum, int oi);
void TryDisarm(int pnum, int oi);
int ItemMiscIdIdx(int imiscid);
void OperateShrine(int pnum, int oi, int psfx, int psfxCnt);
void OperateSkelBook(int pnum, int oi, DIABOOL sendmsg);
void OperateBookCase(int pnum, int oi, DIABOOL sendmsg);
void OperateDecap(int pnum, int oi, DIABOOL sendmsg);
void OperateArmorStand(int pnum, int oi, DIABOOL sendmsg);
int FindValidShrine(int filter);
void OperateGoatShrine(int pnum, int oi);
void OperateCauldron(int pnum, int oi);
DIABOOL OperateFountains(int pnum, int oi);
void OperateWeaponRack(int pnum, int oi, DIABOOL sendmsg);
void OperateStoryBook(int pnum, int oi);
void OperateLazStand(int pnum, int oi);
void OperateObject(int pnum, int oi, BOOL TeleFlag);
void SyncOpL1Door(int pnum, int cmd, int oi);
void SyncOpL2Door(int pnum, int cmd, int oi);
void SyncOpL3Door(int pnum, int cmd, int oi);
void SyncOpObject(int pnum, int cmd, int oi);
void BreakCrux(int oi);
void BreakBarrel(int pnum, int oi, int dam, BOOL forcebreak, BOOL sendmsg);
void BreakObject(int pnum, int oi);
void SyncBreakObj(int pnum, int oi);
void SyncL1Doors(int oi);
void SyncCrux(int oi);
void SyncLever(int oi);
void SyncQSTLever(int oi);
void SyncPedistal(int oi);
void SyncL2Doors(int oi);
void SyncL3Doors(int oi);
void SyncObjectAnim(int oi);
void GetObjectStr(int oi);
void operate_lv24_lever();
void objects_454BA8();
void AddUberLever();
BOOL OpenUberLevel(int s);

/* rdata */

extern int bxadd[8];
extern int byadd[8];
extern const char *const shrinestrs[NUM_SHRINETYPE];
extern char shrinemin[NUM_SHRINETYPE];
extern char shrinemax[NUM_SHRINETYPE];
extern BYTE shrineavail[NUM_SHRINETYPE];
extern const char *const StoryBookName[];
extern int StoryText[3][3];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJECTS_H__ */
