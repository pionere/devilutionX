/**
 * @file init.h
 *
 * Interface of routines for initializing the environment, disable screen saver, load MPQ.
 */
#ifndef __INIT_H__
#define __INIT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbActive;
extern WNDPROC CurrentWndProc;
#ifdef MPQONE
extern HANDLE diabdat_mpq;
#else
extern HANDLE diabdat_mpqs[NUM_MPQS];
#endif

void init_cleanup();
void init_archives();
void init_create_window();
void MainWndProc(UINT Msg);
WNDPROC SetWindowProc(WNDPROC NewProc);

extern char gszProductName[MAX_SEND_STR_LEN];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INIT_H__ */
