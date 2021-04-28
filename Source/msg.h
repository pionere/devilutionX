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
extern _msg_mode geBufferMsgs;
extern char gbNetMsg[MAX_SEND_STR_LEN];

void msg_send_drop_pkt(int pnum, int reason);
bool msg_wait_resync();
void run_delta_info();
void DeltaExportData(int pnum);
void delta_init();
bool delta_portal_inited(int i);
bool delta_quest_inited(int i);
void DeltaAddItem(int ii);
void DeltaSaveLevel();
void DeltaLoadLevel();
void NetSendCmd(bool bHiPri, BYTE bCmd);
void NetSendCmdGolem(BYTE mx, BYTE my, BYTE dir, BYTE menemy, int hp, BYTE cl);
void NetSendCmdMonstKill(int mnum, int pnum);
void NetSendCmdLoc(bool bHiPri, BYTE bCmd, BYTE x, BYTE y);
void NetSendCmdLocParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1);
void NetSendCmdLocParam2(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1, WORD wParam2);
void NetSendCmdLocBParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE wParam1);
void NetSendCmdLocBParam2(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2);
void NetSendCmdLocBParam3(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2, BYTE bParam3);
void NetSendCmdParam1(bool bHiPri, BYTE bCmd, WORD wParam1);
void NetSendCmdParam2(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2);
void NetSendCmdParam3(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2, WORD wParam3);
void NetSendCmdWBParam4(bool bHiPri, BYTE bCmd, WORD wParam1, BYTE wParam2, BYTE wParam3, BYTE wParam4);
void NetSendCmdBParam1(bool bHiPri, BYTE bCmd, BYTE bParam1);
void NetSendCmdBParam2(bool bHiPri, BYTE bCmd, BYTE bParam1, BYTE bParam2);
void NetSendCmdBParam3(bool bHiPri, BYTE bCmd, BYTE bParam1, BYTE bParam2, BYTE bParam3);
void NetSendCmdGItem(bool bHiPri, BYTE bCmd, BYTE mast, BYTE pnum, BYTE ii);
void NetSendCmdPItem(bool bHiPri, BYTE bCmd, ItemStruct *is, BYTE x, BYTE y);
void NetSendCmdDItem(bool bHiPri, int ii);
void NetSendCmdChItem(ItemStruct *is, BYTE bLoc);
void NetSendCmdDelItem(BYTE bLoc);
void NetSendCmdQuest(BYTE q, bool extOnly);
void NetSendCmdDwParam2(bool bHiPri, BYTE bCmd, DWORD dwParam1, DWORD dwParam2);
void NetSendCmdDwParam3(bool bHiPri, BYTE bCmd, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
void NetSendCmdString(unsigned int pmask);
void delta_close_portal(int pnum);
unsigned ParseCmd(int pnum, TCmd *pCmd);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MSG_H__ */
