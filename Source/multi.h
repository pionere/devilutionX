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

extern const char* szGameName;
extern const char* szGamePassword;
extern bool gbSomebodyWonGameKludge;
extern BYTE gbActivePlayers;
extern bool gbSelectProvider;
extern bool gbSelectHero;
extern BYTE gbGameMode;
extern BYTE gbDeltaSender;
extern uint32_t guDeltaTurn;
extern unsigned guSendGameDelta;
extern uint32_t gdwGameLogicTurn;
extern unsigned player_state[MAX_PLRS];

void NetSendChunk(BYTE* pbMsg, BYTE bLen);
void multi_send_direct_msg(unsigned pmask, BYTE* src, BYTE len);
void multi_send_large_direct_msg(int pnum, BYTE bCmd, BYTE* pbSrc, unsigned dwLen);
void multi_process_msgs();
bool multi_handle_turn();
void multi_process_turn(SNetTurnPkt *turn);
void multi_player_left(int pnum, int reason);
void multi_disband_team(int pnum);
bool multi_check_timeout();
void multi_rnd_seeds();
void NetClose();
bool NetInit(bool bSinglePlayer);
void multi_init_buffers();
void multi_recv_plrinfo_msg(int pnum, TCmdPlrInfoHdr* piHdr);

#define IsLocalGame	(gbGameMode <= 1)
#define IsMultiGame	(gbGameMode >= 1)
#define IsGameSrv	(gbGameMode == 3)

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MULTI_H__ */
