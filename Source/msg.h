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
void LevelDeltaExport(uint32_t turn);
// load level info from level-delta
void LevelDeltaLoad();
void msg_send_drop_plr(int pnum, BYTE reason);
// download game-delta
bool DownloadDeltaInfo();
// send game-delta
void DeltaExportData(int pnum, uint32_t turn);
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
void NetSendCmdLocParam1(BYTE bCmd, BYTE x, BYTE y, WORD wParam1);
void NetSendCmdLocBParam1(BYTE bCmd, BYTE x, BYTE y, BYTE wParam1);
void NetSendCmdParam1(BYTE bCmd, WORD wParam1);
void NetSendCmdParam2(BYTE bCmd, WORD wParam1, WORD wParam2);
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
void NetSendCmdItemSkill(int cii, BYTE skill, char from);
/** Attack a location with a given skill using from as a source.
 * @param x: the x coordinate of the target (MAXDUNX)
 * @param y: the y coordinate of the target (MAXDUNY)
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdLocAttack(BYTE x, BYTE y, BYTE skill, char from);
/** Use a spell on a given location using from as a source.
 * @param x: the x coordinate of the target (MAXDUNX)
 * @param y: the y coordinate of the target (MAXDUNY)
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdLocSkill(BYTE x, BYTE y, BYTE skill, char from);
/** Use a disarm on a given location using from as a source.
 * @param x: the x coordinate of the target (MAXDUNX)
 * @param y: the y coordinate of the target (MAXDUNY)
 * @param oi: the index of the target (MAXOBJECTS)
 * @param from: the source of the skill
 */
void NetSendCmdLocDisarm(BYTE x, BYTE y, BYTE oi, char from);
/** Attack a player with a given skill using from as a source.
 * @param pnum: the id of the targeted player
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdPlrAttack(int pnum, BYTE skill, char from);
/** Use a spell on a player using from as a source.
 * @param pnum: the id of the targeted player
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdPlrSkill(int pnum, BYTE skill, char from);
/** Attack a monster with a given skill using from as a source.
 * @param bCmd: the type of the attack
 * @param mnum: the id of the targeted monster
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdMonstAttack(BYTE bCmd, int mnum, BYTE skill, char from);
/** Use a spell on a monster using from as a source.
 * @param mnum: the id of the targeted monster
 * @param skill: the skill to be used
 * @param from: the source of the skill
 */
void NetSendCmdMonstSkill(int mnum, BYTE skill, char from);
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
/** Destroy corpses at a monster
 * @param mnum: the id of the monster
 */
void NetSendCmdMonstCorpse(int mnum);
void NetSendCmdGolem();
void NetSendShrineCmd(BYTE type, int seed);
void NetSendCmdQuest(BYTE q, bool extOnly);
void SendStoreCmd1(unsigned idx, BYTE bStoreId, int value);
void SendStoreCmd2(BYTE bStoreId);
void NetSendCmdString(unsigned int pmask);
void delta_close_portal(int pnum);
unsigned ParseMsg(int pnum, TCmd* pCmd);
unsigned ParseCmd(int pnum, TCmd *pCmd);

void UnPackPkItem(const PkItemStruct*src);
void PackPkItem(PkItemStruct *dest, const ItemStruct *src);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MSG_H__ */
