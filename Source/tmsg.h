/**
 * @file tmsg.h
 *
 * Interface of functionality transmitting chat messages.
 */
#ifndef __TMSG_H__
#define __TMSG_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

int tmsg_get(TPkt *pPkt);
void tmsg_add(TCmdGItem* pMsg);
void tmsg_start();
void tmsg_cleanup();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TMSG_H__ */
