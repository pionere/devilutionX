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
extern _msg_mode geBufferMsgs;
extern char gbNetMsg[MAX_SEND_STR_LEN];

void msg_send_drop_plr(int pnum, int reason);
bool DownloadDeltaInfo();
void RunDeltaPackets();
void DeltaExportData(int pnum, uint32_t turn);
void delta_init();
void DeltaAddItem(int ii);
void DeltaSaveLevel();
void DeltaLoadLevel();
void NetSendCmdSendJoinLevel();
void NetSendCmd(bool bHiPri, BYTE bCmd);
void NetSendCmdLoc(bool bHiPri, BYTE bCmd, BYTE x, BYTE y);
void NetSendCmdLocParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1);
void NetSendCmdLocBParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE wParam1);
void NetSendCmdParam1(bool bHiPri, BYTE bCmd, WORD wParam1);
void NetSendCmdParam2(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2);
void NetSendCmdBParam1(bool bHiPri, BYTE bCmd, BYTE bParam1);
void NetSendCmdBParam2(bool bHiPri, BYTE bCmd, BYTE bParam1, BYTE bParam2);
void NetSendCmdGItem(BYTE bCmd, BYTE ii);
void NetSendCmdPutHoldItem(BYTE bCmd, BYTE x, BYTE y);
void NetSendCmdRespawnItem(int ii);
void NetSendCmdDItem(int ii);
void NetSendCmdChItem(ItemStruct *is, BYTE bLoc);
void NetSendCmdDelItem(BYTE bLoc);
void NetSendCmdLocAttack(BYTE x, BYTE y, int skill, int lvl);
void NetSendCmdLocSkill(BYTE x, BYTE y, int skill, int from, int lvl);
void NetSendCmdPlrAttack(int pnum, int skill, int lvl);
void NetSendCmdPlrSkill(int pnum, int skill, int from, int lvl);
void NetSendCmdPlrDamage(int pnum, unsigned damage);
void NetSendCmdMonstAttack(BYTE bCmd, int mnum, int skill, int lvl);
void NetSendCmdMonstSkill(int mnum, int skill, int from, int lvl);
void NetSendCmdMonstDamage(int mnum, int hitpoints);
void NetSendCmdMonstKill(int mnum, int pnum);
void NetSendCmdGolem(BYTE mx, BYTE my, BYTE dir, BYTE menemy, int hp, BYTE cl);
void NetSendCmdQuest(BYTE q, bool extOnly);
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
