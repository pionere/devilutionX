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

extern BYTE sgbNetUpdateRate;
extern unsigned gdwDeltaBytesSec;
extern uint32_t gdwTurnsInTransit;
extern uint32_t* glpMsgTbl[MAX_PLRS];
extern unsigned gdwLargestMsgSize;
extern unsigned gdwNormalMsgSize;

void nthread_terminate_game(const char *pszFcn);
uint32_t nthread_send_and_recv_turn(uint32_t cur_turn, int turn_delta);
bool nthread_recv_turns(bool *received);
void nthread_set_turn_upper_bit();
void nthread_start(bool set_turn_upper_bit);
void nthread_cleanup();
void nthread_ignore_mutex(bool bStart);
bool nthread_has_500ms_passed();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __NTHREAD_H__ */
