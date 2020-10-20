/**
 * @file towners.h
 *
 * Interface of functionality for loading and spawning towners.
 */
#ifndef __TOWNERS_H__
#define __TOWNERS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern TownerStruct towner[NUM_TOWNERS];

int GetActiveTowner(int tnum);
void SetTownerGPtrs(BYTE *pData, BYTE **pAnim);
void NewTownerAnim(int tnum, BYTE *pAnim, int numFrames, int Delay);
void InitTownerInfo(int tnum, int w, int sel, int type, int x, int y, int ao, int tp);
void InitQstSnds(int tnum);
void InitSmith();
void InitBarOwner();
void InitTownDead();
void InitWitch();
void InitBarmaid();
void InitBoy();
void InitHealer();
void InitTeller();
void InitDrunk();
void InitCows();
#ifdef HELLFIRE
void InitFarmer();
void InitCowFarmer();
void InitGirl();
#endif
void InitTowners();
void FreeTownerGFX();
void TownCtrlMsg(int tnum);
void TownBlackSmith();
void TownBarOwner();
void TownDead();
void TownHealer();
void TownStory();
void TownDrunk();
void TownBoy();
void TownWitch();
void TownBarMaid();
void TownCow();
#ifdef HELLFIRE
void TownFarmer();
void TownCowFarmer();
void TownGirl();
#endif
void ProcessTowners();
ItemStruct *PlrHasItem(int pnum, int item, int *outidx);
void TownerTalk(int talk);
void TalkToTowner(int pnum, int tnum);
void CowSFX(int pnum);

/* data */

extern QuestTalkData Qtalklist[];
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNERS_H__ */
