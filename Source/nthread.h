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
extern uint32_t sgbSentThisCycle;
extern const unsigned gdwLargestMsgSize;
extern const unsigned gdwNormalMsgSize;

/*
 * Send a new turn to the other players.
 * (data does not matter as long as it is a valid address)
 */
void nthread_send_turn(BYTE* data = &gbNetUpdateRate, unsigned size = 0);
int nthread_recv_turns();
void nthread_start();
void nthread_cleanup();
void nthread_run();
void nthread_finish();
bool nthread_has_50ms_passed();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __NTHREAD_H__ */
