/**
 * @file error.h
 *
 * Interface of in-game message functions.
 */
#ifndef __ERROR_H__
#define __ERROR_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t msgdelay;
extern BYTE currmsg;

void InitDiabloMsg(BYTE e);
void ClrDiabloMsg();
void DrawDiabloMsg();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ERROR_H__ */
