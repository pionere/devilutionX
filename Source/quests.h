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

extern bool gbQuestlog;
extern int ReturnLvlX;
extern int ReturnLvlY;
extern int ReturnLvl;
extern BYTE TownWarps;
extern BYTE WaterDone;
extern BYTE DungMsgs;
extern BYTE DungMsgs2;
extern uint32_t LvlVisited;
extern QuestStruct quests[NUM_QUESTS];

#define LEVEL_MASK(x)			((uint32_t)1 << (x))
#define IsLvlVisited(x)			((LvlVisited & LEVEL_MASK(x)) != 0)

void InitQuests();
void InitQuestGFX();
void FreeQuestGFX();
void CheckQuests();
int ForceQuests();
bool QuestStatus(int qn);
void CheckQuestKill(int mnum, bool sendmsg);
void DRLG_CheckQuests();
void SetReturnLvlPos();
void GetReturnLvlPos();
void LoadPWaterPalette();
void ResyncQuests();
void DrawQuestLog();
void StartQuestlog();
void QuestlogUp();
void QuestlogDown();
void QuestlogEnter();
void CheckQuestlog();
void SetMultiQuest(int qn, int qa, int qlog, int qvar);
void PlayDungMsgs();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTS_H__ */
