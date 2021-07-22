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
extern BYTE *pQLogCel;
extern int ReturnLvlX;
extern int ReturnLvlY;
extern int ReturnLvl;
extern QuestStruct quests[NUM_QUESTS];

void InitQuests();
void InitQuestGfx();
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

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTS_H__ */
