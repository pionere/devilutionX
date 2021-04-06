/**
 * @file multi.h
 *
 * Interface of functions for keeping multiplayer games in sync.
 */
#ifndef __MULTI_H__
#define __MULTI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbSomebodyWonGameKludge;
extern WORD sgwPackPlrOffsetTbl[MAX_PLRS];
extern BYTE gbActivePlayers;
extern bool gbGameDestroyed;
extern bool gbSelectProvider;
extern bool gbSelectHero;
extern BYTE gbMaxPlayers;
extern const char *szGameName;
extern const char *szGamePassword;
extern BYTE gbDeltaSender;
extern unsigned player_state[MAX_PLRS];

void NetSendLoPri(BYTE *pbMsg, BYTE bLen);
void NetSendHiPri(BYTE *pbMsg, BYTE bLen);
void multi_send_msg_packet(unsigned int pmask, BYTE *src, BYTE len);
void multi_msg_countdown();
void multi_player_left(int pnum, int reason);
void multi_disband_team(int pnum);
void multi_net_ping();
bool multi_handle_delta();
void multi_process_network_packets();
void multi_send_zero_packet(int pnum, BYTE bCmd, BYTE *pbSrc, DWORD dwLen);
void NetClose();
bool NetInit(bool bSinglePlayer);
void recv_plrinfo(int pnum, TCmdPlrInfoHdr *p, bool recv);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MULTI_H__ */
