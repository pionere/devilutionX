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

extern BYTE gbActivePlayers;
extern bool gbSelectProvider;
extern bool gbSelectHero;
extern bool gbLoadGame;
extern NONETCONST bool gbJoinGame;
extern BYTE gbGameMode;
extern turn_t guDeltaTurn;
extern unsigned guSendGameDelta;
extern unsigned guSendLevelData;
extern unsigned guOweLevelDelta;
extern turn_t guRequestLevelData[MAX_PLRS];
extern turn_t gdwLastGameTurn;
extern turn_t gdwGameLogicTurn;
extern unsigned player_state[MAX_PLRS];

void NetSendChunk(const BYTE* pbMsg, BYTE bLen);
void multi_send_large_msg(unsigned pmask, BYTE bCmd, unsigned wBytes);
void multi_send_direct_msg(unsigned pmask, const BYTE* pbSrc, unsigned wLen);
void multi_process_msgs();
bool multi_handle_turn();
void multi_send_turn_packet();
void multi_pre_process_turn(SNetTurnPkt* turn);
void multi_process_turn(SNetTurnPkt* turn);
void multi_deactivate_player(int pnum);
void multi_disband_team(int pnum);
bool multi_check_timeout();
void multi_rnd_seeds();
void NetClose();
bool NetInit(bool bSinglePlayer);
void multi_recv_plrinfo_msg(int pnum, const TMsgLarge* piMsg);
bool multi_plrinfo_received(int pnum);

#ifdef NONET
#define IsLocalGame true
#define IsMultiGame (gbGameMode >= 1)
#define IsGameSrv   false
#elif defined(HOSTONLY)
#define IsLocalGame false
#define IsMultiGame true
#define IsGameSrv   true
#elif defined(NOHOSTING)
#define IsLocalGame (gbGameMode <= 1)
#define IsMultiGame (gbGameMode >= 1)
#define IsGameSrv   false
#else
#define IsLocalGame (gbGameMode <= 1)
#define IsMultiGame (gbGameMode >= 1)
#define IsGameSrv   (gbGameMode == 3)
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MULTI_H__ */
