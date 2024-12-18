/**
 * @file diablo.h
 *
 * Interface of the main game initialization functions.
 */
#ifndef __DIABLO_H__
#define __DIABLO_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

// current mouse position
extern POS32 MousePos;
extern bool gbWasUiInit;
extern bool gbSndInited;

extern BYTE* pMicrosCel;

void diablo_quit(int exitStatus);
int DiabloMain(int argc, char** argv);
bool PressEscKey();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
