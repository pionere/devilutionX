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

#if USE_MPQONE
extern HANDLE diabdat_mpq;
#else
extern HANDLE diabdat_mpqs[NUM_MPQS];
#endif

void FreeArchives();
void InitArchives();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INIT_H__ */
