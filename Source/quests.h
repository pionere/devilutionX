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

extern BOOL questlog;
extern BYTE *pQLogCel;
extern QuestStruct quests[MAXQUESTS];
extern int gnSetLevelTypeTbl[NUM_SETLVL];
extern int ReturnLvlX;
extern int ReturnLvlY;
extern int ReturnLvl;

void InitQuests();
void CheckQuests();
int ForceQuests();
BOOL QuestStatus(int qn);
void CheckQuestKill(int mnum, BOOL sendmsg);
void DRLG_CheckQuests(int x, int y);
void SetReturnLvlPos();
void GetReturnLvlPos();
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
extern QuestData questlist[MAXQUESTS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTS_H__ */
