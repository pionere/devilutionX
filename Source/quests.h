/**
 * @file quests.cpp
 *
 * Interface of functionality for handling quests.
 */
#ifndef __QUESTS_H__
#define __QUESTS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t glSeedTbl[NUM_LEVELS];
extern DynLevelStruct gDynLevels[NUM_DYNLVLS];
extern BYTE gbTownWarps;
extern BYTE gbWaterDone;
extern uint32_t guLvlVisited;
extern int gnSfxDelay;
extern int gnSfxNum;
extern QuestStruct quests[NUM_QUESTS];

#define LEVEL_MASK(x)   (x < 32 ? (uint32_t)1 << (x) : 0)

static_assert(NUM_FIXLVLS + 2 <= 32, "guLvlVisited can not maintain too many levels.");
// test whether the player has visited the given level (can not be used on the dynamic levels in multiplayer games)
#define IsLvlVisited(x) ((guLvlVisited & ((uint32_t)1 << (x))) != 0)

void InitQuests();
void InitQuestGFX();
void FreeQuestGFX();
void CheckQuests();
bool QuestStatus(int qn);
void CheckQuestKill(int mnum, bool sendmsg);
void DRLG_CheckQuests();
void SetReturnLvlPos();
void GetReturnLvlPos();
void LoadPWaterPalette();
void ResyncBanner();
void ResyncDiablo();
void ResyncQuests();
void DrawQuestLog();
void StartQuestlog();
void QuestlogUp();
void QuestlogDown();
void QuestlogEnter();
void CheckQuestlogClick();
void SetMultiQuest(int qn, int qa, int qlog, int qvar);
void PlayDungMsgs();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTS_H__ */
