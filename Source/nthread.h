/**
 * @file nthread.h
 *
 * Interface of functions for managing game ticks.
 */
#ifndef __NTHREAD_H__
#define __NTHREAD_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BYTE gbNetUpdateRate;
#ifdef ADAPTIVE_NETUPDATE
extern BYTE gbEmptyTurns;
#endif
extern const unsigned gdwDeltaBytesSec;
extern turn_t sgbSentThisCycle;

/*
 * Send a new turn to the other players.
 * (data does not matter as long as it is a valid address)
 */
void nthread_send_turn(BYTE* data = &gbNetUpdateRate, unsigned size = 0);
int nthread_recv_turns();
void nthread_start();
void nthread_cleanup();
void nthread_run();
void nthread_finish(UINT uMsg);
/**
 * @brief return the remaining time (ticks) to the next game-turn
 * @return tick-count till the next game-turn should happen
 */
Sint32 nthread_ticks2gameturn();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __NTHREAD_H__ */
