#include "loopback.h"

#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

static constexpr plr_t PLR_SINGLE = 0;

bool loopback::setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	assert(gameData != NULL);
	return true;
}

SNetMsgPkt* loopback::SNetReceiveMessage()
{
	SNetMsgPkt* pkt = NULL;
	buffer_t* pm;
	unsigned dwLen;

	if (message_queue.empty())
		return pkt;
	pm = &message_queue.front();

	dwLen = pm->size();
	pkt = (SNetMsgPkt*)DiabloAllocPtr(dwLen + sizeof(SNetMsgPkt) - sizeof(pkt->data));
	pkt->nmpPlr = PLR_SINGLE;
	pkt->nmpLen = dwLen;
	memcpy(pkt->data, pm->data(), dwLen);
	message_queue.pop_front();
	return pkt;
}

void loopback::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
#if DEBUG_MODE || DEV_MODE
	if (receiver != SNPLAYER_ALL && receiver != PLR_SINGLE)
		app_error(ERR_APP_LOOPBACK_SENDMSG);
#endif
	message_queue.emplace_back(data, data + size);
}

SNetTurnPkt* loopback::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	SNetTurnPkt* pkt;
	SNetTurn* pt;
	BYTE* data;
	unsigned dwLen;

#if DEBUG_MODE || DEV_MODE
	if (turn_queue.size() != 1)
		app_error(ERR_APP_LOOPBACK_QUEUE_SIZE);
#endif
	pt = &turn_queue.front();
	//      pnum           size
	dwLen = sizeof(BYTE) + sizeof(unsigned);
	dwLen += pt->payload.size();

	pkt = (SNetTurnPkt*)DiabloAllocPtr(dwLen + sizeof(SNetTurnPkt) - sizeof(pkt->data));
	pkt->ntpTurn = pt->turn_id;
	pkt->ntpLen = dwLen;
	data = pkt->data;
	*data = PLR_SINGLE;
	data++;
	//       pnum           size
	dwLen -= sizeof(BYTE) + sizeof(unsigned);
	*(unsigned*)data = dwLen;
	data += sizeof(unsigned);
	memcpy(data, pt->payload.data(), dwLen);
	//data += dwLen;
	turn_queue.pop_front();
	return pkt;
}

turn_status loopback::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
#if DEBUG_MODE || DEV_MODE
	if (turn_queue.empty())
		app_error(ERR_APP_LOOPBACK_POLLTURN);
#endif
	status[PLR_SINGLE] = PCS_CONNECTED | PCS_ACTIVE | PCS_TURN_ARRIVED;
	return TS_ACTIVE; // or TS_LIVE
}

void loopback::SNetSendTurn(turn_t turn, const BYTE* data, unsigned size)
{
	turn_queue.emplace_back(turn, buffer_t(data, data + size));
}

void loopback::SNetLeaveGame()
{
	message_queue.clear();
	turn_queue.clear();
}

void loopback::SNetDropPlayer(int playerid)
{
#if DEBUG_MODE || DEV_MODE
	app_error(ERR_APP_LOOPBACK_DROPPLR);
#endif
}

void loopback::SNetDisconnect()
{
#if DEBUG_MODE || DEV_MODE
	app_error(ERR_APP_LOOPBACK_DROPPLR);
#endif
}

unsigned loopback::SNetGetTurnsInTransit()
{
#if DEBUG_MODE || DEV_MODE
	// should be empty or should have one entry
	if (turn_queue.size() > 1) {
		app_error(ERR_APP_LOOPBACK_TRANSIT);
	}
#endif
	return turn_queue.size();
}

void loopback::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	copy_cstr(gamename, "loopback");
}

} // namespace net
DEVILUTION_END_NAMESPACE
