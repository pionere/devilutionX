/**
 * @file tmsg.cpp
 *
 * Implementation of functionality transmitting chat messages.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static TMsgHdr* sgpTimedMsgHead;

bool tmsg_get(TCmdGItem* pMsg)
{
	TMsgHdr* head;

	head = sgpTimedMsgHead;
	if (head == NULL)
		return false;

	if (!SDL_TICKS_PASSED(SDL_GetTicks(), head->dwTime))
		return false;
	sgpTimedMsgHead = head->pNext;
	//len = head->hdr.bLen;
	//static_assert(sizeof(TurnPkt) >= UCHAR_MAX && sizeof(head->hdr.bLen) == 1, "tmsg_get does not check len before writing to an address.");
	memcpy(pMsg, &head[1], sizeof(TCmdGItem)); //len);
	mem_free_dbg(head);
	return true;
}

void tmsg_add(TCmdGItem* pMsg)
{
	TMsgHdr** tail;

	TMsgHdr* msg = (TMsgHdr*)DiabloAllocPtr(sizeof(TCmdGItem) + sizeof(TMsgHdr));
	msg->pNext = NULL;
	msg->dwTime = SDL_GetTicks() + gnTickDelay * 10;
	//msg->hdr.bLen = sizeof(TCmdGItem);
	memcpy(&msg[1], pMsg, sizeof(TCmdGItem));
	for (tail = &sgpTimedMsgHead; *tail != NULL; tail = &(*tail)->pNext) {
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
	TMsgHdr* next;

	while (sgpTimedMsgHead != NULL) {
		next = sgpTimedMsgHead->pNext;
		MemFreeDbg(sgpTimedMsgHead);
		sgpTimedMsgHead = next;
	}
}

DEVILUTION_END_NAMESPACE
