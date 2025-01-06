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

void EventPlrMsg(const char* pszFmt, ...);
void ReceivePlrMsg(int pnum, const char* pszStr);
//void ClearPlrMsg(int pnum);
void InitPlrMsg();
void DrawPlrMsg(bool onTop);
void StartPlrMsg();
void SetupPlrMsg(int pnum);
void VersionPlrMsg();
void StopPlrMsg();
void plrmsg_HandleMouseReleaseEvent();
bool plrmsg_presskey(int vkey);
void plrmsg_CatToText(const char* inBuf);
void plrmsg_HandleMouseMoveEvent();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLRMSG_H__ */
