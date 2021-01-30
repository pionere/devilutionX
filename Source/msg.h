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

extern BOOL deltaload;
extern BYTE gbBufferMsgs;
extern char gbNetMsg[MAX_SEND_STR_LEN];

void msg_send_drop_pkt(int pnum, int reason);
BOOL msg_wait_resync();
void run_delta_info();
void DeltaExportData(int pnum);
void delta_init();
void delta_kill_monster(int mnum, BYTE x, BYTE y, BYTE bLevel);
void delta_monster_hp(int mnum, int hp, BYTE bLevel);
void delta_sync_monster(const TSyncMonster *pSync, BYTE bLevel);
BOOL delta_portal_inited(int i);
BOOL delta_quest_inited(int i);
void DeltaAddItem(int ii);
void DeltaSaveLevel();
void DeltaLoadLevel();
void NetSendCmd(BOOL bHiPri, BYTE bCmd);
void NetSendCmdGolem(BYTE mx, BYTE my, BYTE dir, BYTE menemy, int hp, BYTE cl);
void NetSendCmdLoc(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y);
void NetSendCmdLocParam1(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1);
void NetSendCmdLocParam2(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1, WORD wParam2);
void NetSendCmdLocBParam3(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE wParam1, BYTE wParam2, BYTE wParam3);
void NetSendCmdParam1(BOOL bHiPri, BYTE bCmd, WORD wParam1);
void NetSendCmdParam2(BOOL bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2);
void NetSendCmdParam3(BOOL bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2, WORD wParam3);
void NetSendCmdWBParam4(BOOL bHiPri, BYTE bCmd, WORD wParam1, BYTE wParam2, BYTE wParam3, BYTE wParam4);
void NetSendCmdBParam1(BOOL bHiPri, BYTE bCmd, BYTE bParam1);
void NetSendCmdQuest(BOOL bHiPri, BYTE q);
void NetSendCmdGItem(BOOL bHiPri, BYTE bCmd, BYTE mast, BYTE pnum, BYTE ii);
void NetSendCmdPItem(BOOL bHiPri, BYTE bCmd, ItemStruct *is, BYTE x, BYTE y);
void NetSendCmdChItem(BOOL bHiPri, ItemStruct *is, BYTE bLoc);
void NetSendCmdDelItem(BOOL bHiPri, BYTE bLoc);
void NetSendCmdDItem(BOOL bHiPri, int ii);
void NetSendCmdDwParam2(BOOL bHiPri, BYTE bCmd, DWORD dwParam1, DWORD dwParam2);
void NetSendCmdString(unsigned int pmask);
void delta_close_portal(int pnum);
DWORD ParseCmd(int pnum, TCmd *pCmd);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MSG_H__ */
