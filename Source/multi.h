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
void multi_send_direct_msg(unsigned pmask, BYTE* src, BYTE len);
void multi_parse_turns();
void multi_player_left(int pnum, int reason);
void multi_disband_team(int pnum);
void multi_net_ping();
bool multi_handle_turn();
void multi_process_network_packets();
void multi_send_large_direct_msg(int pnum, BYTE bCmd, BYTE* pbSrc, unsigned dwLen);
void NetClose();
bool NetInit(bool bSinglePlayer);
void multi_recv_plrinfo_msg(int pnum, TCmdPlrInfoHdr* piHdr);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MULTI_H__ */
