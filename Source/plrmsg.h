/**
 * @file plrmsg.h
 *
 * Interface of functionality for printing the ingame chat messages.
 */
#ifndef __PLRMSG_H__
#define __PLRMSG_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbTalkflag;

void plrmsg_delay(bool delay);
#if DEV_MODE
void ErrorPlrMsg(const char* pszMsg);
#endif
void EventPlrMsg(const char* pszFmt, ...);
void ReceivePlrMsg(int pnum, const char* pszStr);
//void ClearPlrMsg(int pnum);
void InitPlrMsg();
void DrawPlrMsg(bool onTop);
void StartPlrMsg();
void SetupPlrMsg(int pnum, bool shift);
void StopPlrMsg();
bool plrmsg_presskey(int vkey);
bool plrmsg_presschar(int vkey);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLRMSG_H__ */
