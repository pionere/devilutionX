/**
 * @file tmsg.cpp
 *
 * Implementation of functionality transmitting chat messages.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static TMsg *sgpTimedMsgHead;

int tmsg_get(TPkt *pPkt)
{
	int len;
	TMsg *head;

	if (sgpTimedMsgHead == NULL)
		return 0;

	if ((int)(sgpTimedMsgHead->hdr.dwTime - SDL_GetTicks()) >= 0)
		return 0;
	head = sgpTimedMsgHead;
	sgpTimedMsgHead = head->hdr.pNext;
	len = head->hdr.bLen;
	static_assert(sizeof(TPkt) >= UCHAR_MAX && sizeof(head->hdr.bLen) == 1, "tmsg_get does not check len before writing to an address.");
	memcpy(pPkt, head->body, len);
	mem_free_dbg(head);
	return len;
}

void tmsg_add(TCmdGItem *pMsg)
{
	TMsg **tail;

	TMsg *msg = (TMsg *)DiabloAllocPtr(sizeof(TCmdGItem) + sizeof(TMsg));
	msg->hdr.pNext = NULL;
	msg->hdr.dwTime = SDL_GetTicks() + gnTickDelay * 10;
	msg->hdr.bLen = sizeof(TCmdGItem);
	memcpy(msg->body, pMsg, sizeof(TCmdGItem));
	for (tail = &sgpTimedMsgHead; *tail; tail = &(*tail)->hdr.pNext) {
		;
	}
	*tail = msg;
}

void tmsg_start()
{
	assert(sgpTimedMsgHead == NULL);
}

void tmsg_cleanup()
{
	TMsg *next;

	while (sgpTimedMsgHead != NULL) {
		next = sgpTimedMsgHead->hdr.pNext;
		MemFreeDbg(sgpTimedMsgHead);
		sgpTimedMsgHead = next;
	}
}

DEVILUTION_END_NAMESPACE
