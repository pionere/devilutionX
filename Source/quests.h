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
extern QuestStruct quests[NUM_QUESTS];
extern int gnSetLevelTypeTbl[NUM_SETLVL];
extern int ReturnLvlX;
extern int ReturnLvlY;
extern int ReturnLvl;

void InitQuests();
void CheckQuests();
int ForceQuests();
bool QuestStatus(int qn);
void CheckQuestKill(int mnum, bool sendmsg);
void DRLG_CheckQuests(int x, int y);
void SetReturnLvlPos();
void GetReturnLvlPos();
void LoadPWaterPalette();
void ResyncMPQuests();
void ResyncQuests();
void DrawQuestLog();
void StartQuestlog();
void QuestlogUp();
void QuestlogDown();
void QuestlogEnter();
void CheckQuestlog();
void SetMultiQuest(int qn, int qa, int qlog, int qvar);

/* rdata */
extern const QuestData questlist[NUM_QUESTS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTS_H__ */
