/**
 * @file msg.h
 *
 * Interface of function for sending and reciving network messages.
 */
#ifndef __MSG_H__
#define __MSG_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool deltaload;
extern DeltaData gsDeltaData;
extern _msg_mode geBufferMsgs;
extern char gbNetMsg[MAX_SEND_STR_LEN];

// send level-delta
void LevelDeltaExport();
// load level info from level-delta
void LevelDeltaLoad();
// download game-delta
bool DownloadDeltaInfo();
// send game-delta
void DeltaExportData(int pmask);
void delta_init();
// add items which are part of the dungeon to the game-delta
void DeltaAddItem(int ii);
// save automap to game-delta
void DeltaSaveLevel();
// load level info from game-delta
void DeltaLoadLevel();
void NetSendCmdJoinLevel();
void NetSendCmd(BYTE bCmd);
void NetSendCmdLoc(BYTE bCmd, BYTE x, BYTE y);
void NetSendCmdLocParam1(BYTE bCmd, BYTE x, BYTE y, uint16_t wParam1);
void NetSendCmdLocBParam1(BYTE bCmd, BYTE x, BYTE y, BYTE bParam1);
void NetSendCmdLocBParam2(BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2);
void NetSendCmdParam1(BYTE bCmd, uint16_t wParam1);
void NetSendCmdParamBW(BYTE bCmd, BYTE bParam1, uint16_t wParam2);
void NetSendCmdBParam1(BYTE bCmd, BYTE bParam1);
void NetSendCmdBParam2(BYTE bCmd, BYTE bParam1, BYTE bParam2);
void NetSendCmdGItem(BYTE bCmd, BYTE ii);
void NetSendCmdPutItem(BYTE x, BYTE y);
void NetSendCmdSpawnItem(bool flipFlag);
void NetSendCmdDelItem(BYTE bLoc);
/** Use a spell on an item using from as a source.
 * @param cii: the index of the item in the inventory
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdItemSkill(BYTE cii, BYTE skill, int8_t from);
/** Use a spell/skill on a given location using from as a source.
 * @param x: the x coordinate of the target (MAXDUNX)
 * @param y: the y coordinate of the target (MAXDUNY)
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdLocSkill(BYTE x, BYTE y, BYTE skill, int8_t from);
/** Use a disarm on a given location using from as a source.
 * @param x: the x coordinate of the target (MAXDUNX)
 * @param y: the y coordinate of the target (MAXDUNY)
 * @param oi: the index of the target (MAXOBJECTS)
 * @param from: the source of the skill
 */
void NetSendCmdLocDisarm(BYTE x, BYTE y, BYTE oi, int8_t from);
/** Use a spell/skill on a player using from as a source.
 * @param pnum: the id of the targeted player
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdPlrSkill(int pnum, BYTE skill, int8_t from);
/** Use a spell/skill on a monster using from as a source.
 * @param mnum: the id of the targeted monster
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdMonSkill(int mnum, BYTE skill, int8_t from);
/** Update monster hp after it was damaged.
 * @param mnum: the id of the monster
 * @param hp: the hp of the monster after the damage
 */
void NetSendCmdMonstDamage(int mnum, int hitpoints);
/** Register the death of a monster
 * @param mnum: the id of the monster
 * @param pnum: the id of the player who killed it
 */
void NetSendCmdMonstKill(int mnum, int pnum);
/** Summon a monster
 * @param mnum: the id of the monster
 */
void NetSendCmdMonstSummon(int mnum);
void NetSendCmdGolem();
void NetSendShrineCmd(BYTE type, int seed);
void NetSendCmdQuest(BYTE q, bool extOnly);
void SendStoreCmd1(BYTE idx, BYTE bStoreId, int value);
void SendStoreCmd2(BYTE bStoreId);
/** Initiate level change.
 * @param fom: the type of the level-change (window_messages)
 * @param bLevel: the destination level
 */
void NetSendCmdNewLvl(BYTE fom, BYTE bLevel);
/** Initiate level creation and change.
 * @param seed: the seed of the level
 */
void NetSendCmdCreateLvl(DWORD seed);
void NetSendCmdString(unsigned int pmask);
unsigned ParseMsg(int pnum, TCmd* pCmd);
unsigned ParseCmd(int pnum, TCmd* pCmd);

void UnPackPkItem(const PkItemStruct* src);
void PackPkItem(PkItemStruct* dest, const ItemStruct* src);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MSG_H__ */
