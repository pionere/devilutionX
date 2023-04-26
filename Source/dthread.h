/**
 * @file dthread.h
 *
 * Interface of functions for updating game state from network commands.
 */
#ifndef __DTHREAD_H__
#define __DTHREAD_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NONET
void dthread_remove_player(int pnum);
void dthread_send_delta(int pnum, BYTE cmd, const BYTE* pbSrc, int dwLen);
void dthread_start();
void dthread_cleanup();
#else
inline void dthread_remove_player(int pnum) { };
inline void dthread_send_delta(int pnum, BYTE cmd, const BYTE* pbSrc, int dwLen) { };
inline void dthread_start() { };
inline void dthread_cleanup() { };
#endif // NONET

/* data */
#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DTHREAD_H__ */
